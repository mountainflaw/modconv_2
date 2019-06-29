/*
 *   Copyright (c) 2019, Red                                                             *
 *   All rights reserved.                                                                *
 *                                                                                       *
 *   Redistribution and use in source and binary forms, with or without                  *
 *   modification, are permitted provided that the following conditions are met:         *
 *                                                                                       *
 *       * Redistributions of source code must retain the above copyright                *
 *         notice, this list of conditions and the following disclaimer.                 *
 *       * Redistributions in binary form must reproduce the above copyright             *
 *         notice, this list of conditions and the following disclaimer in the           *
 *         documentation and/or other materials provided with the distribution.          *
 *       * Neither the name of the modconv 2 developers nor the                          *
 *         names of its contributors may be used to endorse or promote products          *
 *         derived from this software without specific prior written permission.         *
 *                                                                                       *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND     *
 *   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED       *
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE              *
 *   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY                *
 *   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES          *
 *   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;        *
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND         *
 *   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS       *
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                        *
 */

/*
 * New F3D builder.
 * Differences from the last:
 * 1.) It no longer assumes vertices when loading the vertex buffers, meaning that stray
 * vertices no longer can cause problems with importing models.
 * 2.) The code is a lot cleaner.
 * 3.) The display list builder is no longer its own file since it is just a loop.
 * 4.) The material class has been simplified.
 */

enum RgbaColors { C_RED, C_GRN, C_BLU, C_APH };
enum Layers { LAYER_0, LAYER_1, LAYER_2, LAYER_3, LAYER_4, LAYER_5, LAYER_6, LAYER_7 };

#include "modconv.hxx"
#include "buffer.hxx"
#include "material.hxx"

/* global variabes */

u32 vert     = 0,
    vert2    = 0;
u16 vBuffers = 0,
    vBuffer  = 0;

u8 diffuse[6] = {0xFF, 0xFF, 0xFF, 0x28, 0x28, 0x28}, ambient[3] = {0x66, 0x66, 0x66};

bool fog = false;
u16 fogSettings[6];

static void count_vtx(aiNode* node, const aiScene* scene)
{
    for (u16 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        vert += mesh->mNumFaces * 3;
    }

    for (u16 i = 0; i < node->mNumChildren; i++) {
        count_vtx(node->mChildren[i], scene);
    }
}

/** Add vertices to vertex buffers. */
static void setup_vtx(aiNode *node, const aiScene* scene, s16 scale,
        VertexBuffer* vBuf, const std::string &file, bool uvFlip)
{
    for (u16 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        /* we go by faces instead of verts so we don't accidentally add what we don't need */
        for (u32 j = 0; j < mesh->mNumFaces; j++) {
            for (u8 k = 0; k <= 2; k++) {
                u32 currVtx = mesh->mFaces[j].mIndices[k];

                if (vBuffers == 1) { /* if we only have one buffer, set it to the size of vert so we don't overflow */
                    vBuf[i].bufferSize = vert;
                }

                if (vBuf[vBuffer].isBufferComplete()) {
                    vBuf[vBuffer].vtxCount = 0;
                    vBuffer++;

                    if (vBuffer == vBuffers - 1) { /* set the max amount for the final vbuffer */
                        vBuf[vBuffer].bufferSize = vert - vert2;
                    }
                }

                s16 pos[3];

                pos[AXIS_X] = (s16)(((mesh->mVertices[currVtx].x) * scale) * 0.01);
                pos[AXIS_Y] = (s16)(((mesh->mVertices[currVtx].y) * scale) * 0.01);
                pos[AXIS_Z] = (s16)(((mesh->mVertices[currVtx].z) * scale) * 0.01);

                s16 uv[2] = {0x00};

                /* We have to look at material data so we can multiply the UV data. */
                if (scene->HasMaterials() && mesh->HasTextureCoords(0)) { /* ditto */
                    aiString aiPath;
                    scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &aiPath);
                    std::string path = aiPath.data;

                    if (file_exists(path)) { /* absolute */
                        uv[AXIS_X] = mesh->mTextureCoords[0][currVtx].x * 32 * get_dimension(AXIS_X, path);
                        uv[AXIS_Y] = mesh->mTextureCoords[0][currVtx].y * 32 * get_dimension(AXIS_Y, path);
                    } else if (file_exists(get_path(file) + path) && !(is_directory(get_path(file) + path))) { /* relative */
                        uv[AXIS_X] = mesh->mTextureCoords[0][currVtx].x * 32 * get_dimension(AXIS_X, get_path(file) + path);
                        uv[AXIS_Y] = mesh->mTextureCoords[0][currVtx].y * 32 * get_dimension(AXIS_Y, get_path(file) + path);
                    }

                    /* Some formats use flipped UVs. Flip them here if the option was passed. */
                    if (uvFlip) {
                        uv[AXIS_Y] *= -1;
                    }
                }

                s16 rgba[4] = {0xff, 0xff, 0xff, 0xff};

                if (mesh->HasVertexColors(0)) { /* Get around segfault. */
                    rgba[C_RED] = mesh->mColors[0][currVtx].r * 0xff;
                    rgba[C_GRN] = mesh->mColors[0][currVtx].g * 0xff;
                    rgba[C_BLU] = mesh->mColors[0][currVtx].b * 0xff;
                    rgba[C_APH] = mesh->mColors[0][currVtx].a * 0xff;
                }

                aiString aiName;
                scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_NAME, aiName);
                std::string nameStr = aiName.data;

                /*
                 * Add normals based lighting (#SHADE)
                 * or vertex color based off of normals (#NORMCOLOR)
                 */

                if (scene->HasMaterials() && mesh->HasNormals() && (nameStr.find("#LIGHTING") != std::string::npos
                            || nameStr.find("#NORMCOLOR") != std::string::npos)) {

                    rgba[C_RED] = mesh->mNormals[currVtx].x * 127;
                    rgba[C_GRN] = mesh->mNormals[currVtx].y * 127;
                    rgba[C_BLU] = mesh->mNormals[currVtx].z * 127;

                    if (mesh->HasVertexColors(0)) { /* Get around potential segfault. */
                        if (nameStr.find("#REDALPHA") != std::string::npos) {
                            rgba[C_APH] = mesh->mColors[0][currVtx].r * 0xff; /* stomatol wanted this to bake lights */
                        } else {
                            rgba[C_APH] = mesh->mColors[0][currVtx].a * 0xff;
                        }
                    }
                }

                /* Figure out what layer vtx belongs to */
                u8 layer = 1;

                if (scene->HasMaterials()) {
                    std::string layerTags[8] = { "#LAYER_0", "#LAYER_1", "#LAYER_2", "#LAYER_3", "#LAYER_4", "#LAYER_5", "#LAYER_6", "#LAYER_7" };

                    for (u8 i = 0; i < 8; i++) {
                        if (nameStr.find(layerTags[i]) != std::string::npos) {
                            layer = i;
                            break;
                        }
                    }
                }

                vBuf[vBuffer].addVtx(pos[AXIS_X], pos[AXIS_Y], pos[AXIS_Z],
                        uv[AXIS_X], uv[AXIS_Y],
                        rgba[C_RED], rgba[C_GRN], rgba[C_BLU], rgba[C_APH], mesh->mMaterialIndex, layer);
                vert2++;
            }
        }
    }
    for (u16 i = 0; i < node->mNumChildren; i++) {
        setup_vtx(node->mChildren[i], scene, scale, vBuf, file, uvFlip);
    }
}

enum BufferModes {RESET, OPTIMIZE, BUFFER};

/** Function for common vbuffer operations (reset counter, run the optimizer, etc). */
static inline void cycle_vbuffers(VertexBuffer *vBuf, u8 mode, u8 microcode)
{
    switch (mode) {
        case BUFFER:
        for (u16 i = 0; i < vBuffers; i++) {
            vBuf[i].bufferSize = microcode;
        }
        break;

        case OPTIMIZE:
        for (u16 i = 0; i < vBuffers; i++) {
            vBuf[i].optimizeVerts();
        }

        case RESET:
        for (u16 i = 0; i < vBuffers; i++) {
            vBuf[i].vtxCount = 0;
        }
        break;
    }
}

static void write_vtx(const std::string fileOut, const std::string &path, VertexBuffer *vBuf)
{
    std::fstream vtxOut;
    vtxOut.open(fileOut + "/model.s", std::ofstream::out | std::ofstream::app);
    for (u16 i = 0; i < vBuffers; i++) {
        vtxOut << std::endl << labelize(get_filename(fileOut) + "_vertex_" + std::to_string(i)) << std::endl;
        for (u16 j = 0; j < vBuf[i].bufferSize; j++) {
            Vertex vtx = vBuf[i].getVtx();
            if (!vtx.useless) {
                vtxOut << "vertex " << vtx.pos[AXIS_X] << ", "
                                    << vtx.pos[AXIS_Y] << ", "
                                    << vtx.pos[AXIS_Z] << ", "
                                    << vtx.st[AXIS_X]  << ", "
                                    << vtx.st[AXIS_Y]  << ", "
                                    << (u16)((u8)vtx.col[C_RED])  << ", "
                                    << (u16)((u8)vtx.col[C_GRN])  << ", "
                                    << (u16)((u8)vtx.col[C_BLU])  << ", "
                                    << (u16)((u8)vtx.col[C_APH])  << std::endl;
            }
        }
    }
}

static inline std::string hex_string(const u8 hex)
{
    std::stringstream s;
    s << std::hex << (u16)hex;
    return "0x" + s.str();
}

static void configure_materials(const std::string &file, const std::string &fileOut, Material* mat, const aiScene* scene)
{
    for (u16 i = 0; i < scene->mNumMaterials; i++) {
        aiString aiPath, aiName;
        scene->mMaterials[i]->Get(AI_MATKEY_NAME, aiName);
        scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &aiPath);

        mat[i].setName(aiName.data);
        mat[i].setFile(fileOut);

        if (file_exists(aiPath.data)) { /* absolute */
            mat[i].setPath(aiPath.data);
            mat[i].textured = true;
        } else if (file_exists(get_path(file) + aiPath.data) && !(is_directory(get_path(file) + aiPath.data))) { /* relative */
            mat[i].setPath(get_path(file) + aiPath.data);
            mat[i].textured = true;
        }

        std::cout << "Texture (absolute) " << aiPath.data << std::endl;
        std::cout << "Texture (relative) " << get_path(file) + aiPath.data << std::endl;
    }
}

static void write_textures(const std::string &fileOut, Material *mat, const aiScene* scene, bool level)
{
    std::fstream texOut;
    if (level) {
        reset_file(fileOut + "/texture.s");
        texOut.open(fileOut + "/texture.s", std::ofstream::out | std::ofstream::app);
    } else { /* generating an actor */
        texOut.open(fileOut + "/model.s", std::ofstream::out | std::ofstream::app);
    }


    /*
     * Phase 1: Copy lights
     * These should be changable soon.
     */
    texOut << std::endl << get_filename(fileOut) << "_ambient_light:" << std::endl
           << ".byte " + hex_string(ambient[0]) + ", " + hex_string(ambient[1]) + ", " + hex_string(ambient[2]) + ", 0x00, " + hex_string(ambient[0]) + ", " + hex_string(ambient[1]) + ", " + hex_string(ambient[2]) + ", 0x00" << std::endl
           << get_filename(fileOut) << "_diffuse_light:" << std::endl
           << ".byte " + hex_string(diffuse[0]) + ", " + hex_string(diffuse[1]) + ", " + hex_string(diffuse[2]) + ", 0x00, " + hex_string(diffuse[0]) + ", " + hex_string(diffuse[1]) + ", " + hex_string(diffuse[2]) + ", 0x00" << std::endl
           << ".byte " + hex_string(diffuse[3]) + ", " + hex_string(diffuse[4]) + ", " + hex_string(diffuse[5]) + ", 0x00, 0x00, 0x00, 0x00, 0x00" << std::endl;
    /* Phase 2 - Find redundant textures */
    for (u16 i = 0; i < scene->mNumMaterials; i++) {
        for (u16 j = 0; j < scene->mNumMaterials; j++) {
            if (mat[i].getPath().compare(mat[j].getPath()) == 0 && j > i) {
                mat[j].useless = true;
            }
        }
    }

    /* Phase 3: Write and copy textures */
    for (u16 i = 0; i < scene->mNumMaterials; i++) {
        if (!mat[i].useless && mat[i].textured) {
            texOut << std::endl;
            texOut << labelize(mat[i].getFileNameNoExtension()) << std::endl;
            if (level) {
                texOut << ".incbin " << R"(")" << "levels/" << get_filename(fileOut) << "/" << mat[i].getFileNameNoExtension() << R"(")" << std::endl;
                if (mat[i].getFileNameNoExtension().find("ci4") != std::string::npos
                        || mat[i].getFileNameNoExtension().find("ci8") != std::string::npos) { /* CI palette */
                    texOut << std::endl << mat[i].getFileNameNoExtension() << "_pal:" << std::endl;
                    texOut << ".incbin " << R"(")" << "levels/" << get_filename(fileOut) << "/" << mat[i].getFileNameNoExtension() << R"(.pal")" << std::endl;
                }
            } else { /* generating an actor */
                texOut << ".incbin " << R"(")" << "actors/" << get_filename(fileOut) << "/" << mat[i].getFileNameNoExtension() << R"(")" << std::endl;
                if (mat[i].getFileNameNoExtension().find("ci4") != std::string::npos
                        || mat[i].getFileNameNoExtension().find("ci8") != std::string::npos) { /* CI palette */
                    texOut << std::endl << mat[i].getFileNameNoExtension() << "_pal:" << std::endl;
                    texOut << ".incbin " << R"(")" << "actors/" << get_filename(fileOut) << "/" << mat[i].getFileNameNoExtension() << R"(.pal")" << std::endl;
                }
            }

            if (file_exists(mat[i].getPath())) {
                remove_file(fileOut + "/" + get_filename(mat[i].getPath()));
            }

            copy_file(mat[i].getPath(), fileOut + "/" + get_filename(mat[i].getPath()));
        }
    }
}

/** Write display list commands to file. */
/*
 * PROCESS:
 * 1.) Setup (Add glabel and disable G_LIGHTING)
 * 2.) Check if materials are different by looking at the mesh ids a vertex corresponds to.
 * 3.) Check if TRI2 is possible within either the vertex buffer range AND if it doesnt go into
 * a vertex with a different material.
 * 4.) End displaylist after all of that crap is done.
 */

static void write_display_list(const std::string &fileOut, VertexBuffer* vBuf, Material* mat)
{
    std::string layers[8] = {
        "G_RM_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF",        /* layer 0 */
        "G_RM_AA_ZB_OPA_DECAL, G_RM_AA_ZB_OPA_INTER",   /* layer 1 */
        "G_RM_AA_ZB_TEX_EDGE, G_RM_AA_ZB_XLU_SURF",     /* layer 2 */
        "G_RM_AA_ZB_XLU_DECAL, G_RM_AA_ZB_XLU_INTER",   /* layer 3 */
        "G_RM_ZB_OPA_SURF2, G_RM_AA_ZB_OPA_SURF2",      /* layer 4 */
        "G_RM_AA_ZB_OPA_DECAL2, G_RM_AA_ZB_OPA_INTER2", /* layer 5 */
        "G_RM_AA_ZB_TEX_EDGE2, G_RM_AA_ZB_XLU_SURF2",   /* layer 6 */
        "G_RM_AA_ZB_XLU_DECAL2, G_RM_AA_ZB_XLU_INTER2", /* layer 7 */
    };

    bool oldGeo[5] = {0x00};
    std::fstream gfxOut;
    gfxOut.open(fileOut + "/model.s", std::ofstream::out | std::ofstream::app);
    s16 currMat = -1;
    u8 currLayer = 1;
    gfxOut << std::endl << "glabel " << get_filename(fileOut) << "_dl" << std::endl
           << "gsSPClearGeometryMode G_LIGHTING" << std::endl;
    if (fog) {
        currLayer = vBuf[0].getVtxLayer(); /* Fog only allows two rendermode settings. */

        gfxOut << "gsDPSetCycleType G_CYC_2CYCLE" << std::endl;

        if (currLayer > 1) { /* transparency layers */
            gfxOut << "gsDPSetRenderMode G_RM_FOG_SHADE_A, G_RM_AA_ZB_XLU_SURF2" << std::endl;
        } else { /* opaque layers */
            gfxOut << "gsDPSetRenderMode G_RM_FOG_SHADE_A, G_RM_AA_ZB_OPA_SURF2" << std::endl;
        }

        gfxOut << "gsSPSetGeometryMode G_FOG" << std::endl
               << "gsSPFogPosition " << fogSettings[4] << ", " << fogSettings[5] << std::endl
               << "gsDPSetFogColor " << fogSettings[0] << ", " << fogSettings[1] << ", "
                                     << fogSettings[2] << ", " << fogSettings[3] << std::endl;

    }
    for (u16 i = 0; i < vBuffers; i++) {

        if (vBuf[i].getVtxMat() != currMat) {
            currMat = vBuf[i].getVtxMat();
            gfxOut << "/* " << mat[currMat].getName() << " */" << std::endl
                            << mat[currMat].getMaterial(oldGeo);
        }

        if (vBuf[i].getVtxLayer() != currLayer) {
            currLayer = vBuf[i].getVtxLayer();

            if (fog) {
                if (currLayer > 1) {
                    gfxOut << "gsDPSetRenderMode G_RM_FOG_SHADE_A, G_RM_AA_ZB_XLU_SURF2" << std::endl;
                } else {
                    gfxOut << "gsDPSetRenderMode G_RM_FOG_SHADE_A, G_RM_AA_ZB_OPA_SURF2" << std::endl;
                }
            } else {
                gfxOut << "gsDPSetRenderMode " << layers[currLayer] << std::endl;
            }
        }

        gfxOut << "gsSPVertex " <<  get_filename(fileOut) << "_vertex_" << i
               << " " << std::to_string(vBuf[i].loadSize) << ", 0" << std::endl;
        extern_data(fileOut, "extern Vertex *" + get_filename(fileOut) + "_vertex_" + std::to_string(i) + "[" + std::to_string(vBuf[i].loadSize) + "];");
        while (!vBuf[i].isBufferComplete()) {
            if (vBuf[i].getVtxMat() != currMat) {
                currMat = vBuf[i].getVtxMat();
                gfxOut << "/* " << mat[currMat].getName() << " */" << std::endl
                                << mat[currMat].getMaterial(oldGeo);
            }

            if (vBuf[i].getVtxLayer() != currLayer) {
                currLayer = vBuf[i].getVtxLayer();

                if (fog) {
                    if (currLayer > 1) {
                        gfxOut << "gsDPSetRenderMode G_RM_FOG_SHADE_A, G_RM_AA_ZB_XLU_SURF2" << std::endl;
                    } else {
                        gfxOut << "gsDPSetRenderMode G_RM_FOG_SHADE_A, G_RM_AA_ZB_OPA_SURF2" << std::endl;
                    }
                } else {
                    gfxOut << "gsDPSetRenderMode " << layers[currLayer] << std::endl;
                }
            }

            if (vBuf[i].canTri2()) {
                u16 triTwo[6] = { vBuf[i].getVtxIndex(), vBuf[i].getVtxIndex(), vBuf[i].getVtxIndex(),
                                  vBuf[i].getVtxIndex(), vBuf[i].getVtxIndex(), vBuf[i].getVtxIndex() };

                gfxOut << "gsSP2Triangles "
                       << triTwo[0] << ", "
                       << triTwo[1] << ", "
                       << triTwo[2] << ", 0x00, "
                       << triTwo[3] << ", "
                       << triTwo[4] << ", "
                       << triTwo[5] << ", 0x00"
                       << std::endl;
            } else {
                u16 triOne[3] = { vBuf[i].getVtxIndex(), vBuf[i].getVtxIndex(), vBuf[i].getVtxIndex() };

                gfxOut << "gsSP1Triangle "
                       << triOne[0] << ", "
                       << triOne[1] << ", "
                       << triOne[2] << ", 0x00" << std::endl;
            }
        }
    }

    gfxOut << "gsSPTexture -1, -1, 0, 0, 0" << std::endl
           << "gsDPPipeSync" << std::endl
           << "gsDPSetCombineMode1Cycle G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_SHADE, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE" << std::endl
           << "gsSPSetGeometryMode G_LIGHTING" << std::endl
           << "gsDPSetTextureLUT G_TT_NONE" << std::endl;

    if (fog) { /* Clear fog settings */
        gfxOut << "gsDPSetCylceType G_CYC_1CYCLE" << std::endl
               << "gsDPSetRenderMode G_RM_AA_ZB_OPA_SURF, G_RM_NOOP2" << std::endl
               << "gsSPClearGeometryMode G_FOG" << std::endl;
    }

    gfxOut << "gsSPEndDisplayList" << std::endl;
}


//#define AI_CONFIG_PP_PTV_NORMALIZE   "PP_PTV_NORMALIZE"
/** Main function for the F3D build process. */
void f3d_main(const std::string &file, const std::string &fileOut, s16 scale, u8 microcode, bool level, bool uvFlip)
{
    Assimp::Importer importer;

    /* We don't use ASSIMP's built in tristripping because of the vertex buffer. */
    const aiScene* scene = importer.ReadFile(file, aiProcess_ValidateDataStructure | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_GenUVCoords);

    reset_file(fileOut + "/model.s");
    count_vtx(scene->mRootNode, scene);

    vBuffers = vert / microcode;

    if (vert % microcode > 0) { /* is there a trailing vbuffer? */
        vBuffers++;
    }

    info_message("Vertices " + std::to_string(vert));
    info_message("Triangles " + std::to_string(vert / 3));

    VertexBuffer vBuf[vBuffers];
    cycle_vbuffers(vBuf, BUFFER, microcode);

    setup_vtx(scene->mRootNode, scene, scale, vBuf, file, uvFlip);

    /* Materials */
    Material mat[scene->mNumMaterials];
    configure_materials(file, fileOut, mat, scene);
    write_textures(fileOut, mat, scene, level);

    cycle_vbuffers(vBuf, OPTIMIZE, 0);
    write_vtx(fileOut, "", vBuf);
    cycle_vbuffers(vBuf, RESET, 0);
    write_display_list(fileOut, vBuf, mat);
}
