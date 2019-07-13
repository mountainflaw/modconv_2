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
#include "displaylist.hxx"

/* global variabes */

u32 vert     = 0,
    vert2    = 0;
u16 vBuffers = 0,
    vBuffer  = 0;
u8  layers   = 0;
bool setLayer[8] = { false };

u8 diffuse[6] = {0xFF, 0xFF, 0xFF, 0x28, 0x28, 0x28}, ambient[3] = {0x66, 0x66, 0x66};

bool fog = false;
u16 fogSettings[6]; /* rgba near far */
enum FogSettings { FOG_RED, FOG_GREEN, FOG_BLUE, FOG_NEAR, FOG_FAR };

static void inspect_vtx(aiNode* node, const aiScene* scene) {
    for (u16 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        vert += mesh->mNumFaces * 3;
    }

    for (u16 i = 0; i < node->mNumChildren; i++) {
        inspect_vtx(node->mChildren[i], scene);
    }
}

namespace uvutil {
    /** Resets UVs to zero if they overflow. */
    static inline s16 reset_uv(s32 uv) {
        if (uv > 32767 || uv < -32768) {
            return 0;
        } else {
            return uv;
        }
    }
}

/** Add vertices to vertex buffers. */
static void setup_vtx(aiNode *node, const aiScene* scene, s16 scale,
        VertexBuffer* vBuf, const std::string &file, bool uvFlip, Material* mat) {
    bool untextured = false;
    for (u16 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        /* we go by faces instead of verts so we don't accidentally add what we don't need */
        for (u32 j = 0; j < mesh->mNumFaces; j++) {
            for (u8 k = 0; k < 3; k++) {
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

                pos[AXIS_X] = (s16)(((mesh->mVertices[currVtx].x) * scale) * scaling_hack());
                pos[AXIS_Y] = (s16)(((mesh->mVertices[currVtx].y) * scale) * scaling_hack());
                pos[AXIS_Z] = (s16)(((mesh->mVertices[currVtx].z) * scale) * scaling_hack());

                s16 uv[2] = {0x00};

                /* We have to look at material data so we can multiply the UV data. */
                if (scene->HasMaterials() && mesh->HasTextureCoords(0)) { /* ditto */
                    aiString aiPath;
                    scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &aiPath);
                    std::string path = aiPath.data;

                    if (mat[mesh->mMaterialIndex].textured) { /* absolute */
                        uv[AXIS_X] = uvutil::reset_uv(mesh->mTextureCoords[0][currVtx].x * 32 * mat[mesh->mMaterialIndex].getDimension(AXIS_X));
                        uv[AXIS_Y] = uvutil::reset_uv(mesh->mTextureCoords[0][currVtx].y * 32 * mat[mesh->mMaterialIndex].getDimension(AXIS_Y));
                    } else {
                        untextured = true;
                    }

                    /* Some formats use flipped UVs. Flip them here if the option was passed. */
                    if (!uvFlip) {
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
                            || nameStr.find("#NORMCOLOR") != std::string::npos || untextured)) {

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
                            setLayer[i] = true;
                            break;
                        } else {
                            setLayer[1] = true;
                        }
                    }
                } else {
                    setLayer[1] = true;
                }

                vBuf[vBuffer].addVtx(pos[AXIS_X], pos[AXIS_Y], pos[AXIS_Z],
                        uv[AXIS_X], uv[AXIS_Y],
                        rgba[C_RED], rgba[C_GRN], rgba[C_BLU], rgba[C_APH], mesh->mMaterialIndex, layer);
                vert2++;
            }
        }
    }
    for (u16 i = 0; i < node->mNumChildren; i++) {
        setup_vtx(node->mChildren[i], scene, scale, vBuf, file, uvFlip, mat);
    }
}

enum BufferModes {RESET, OPTIMIZE, BUFFER};

/** Function for common vbuffer operations (reset counter, run the optimizer, etc). */
static inline void cycle_vbuffers(VertexBuffer *vBuf, u8 mode, u8 microcode) {
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

static void write_vtx(const std::string fileOut, const std::string &path, VertexBuffer *vBuf) {
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

static inline std::string hex_string(const u8 hex) {
    std::stringstream s;
    s << std::hex << (u16)hex;
    return "0x" + s.str();
}

static void configure_materials(const std::string &file, const std::string &fileOut, Material* mat, const aiScene* scene) {
    for (u16 i = 0; i < scene->mNumMaterials; i++) {
        aiString aiPath, aiName;
        scene->mMaterials[i]->Get(AI_MATKEY_NAME, aiName);
        scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &aiPath);

        mat[i].setFile(fileOut);

        if (file_exists(aiPath.data)) { /* absolute */
            mat[i].setPath(aiPath.data);
            mat[i].textured = true;
        } else if (file_exists(get_path(file) + aiPath.data) && !(is_directory(get_path(file) + aiPath.data))) { /* relative */
            mat[i].setPath(get_path(file) + aiPath.data);
            mat[i].textured = true;
        }

        /* Untextured models are always shaded. */
        std::string lightingToggle = "";
        if (!mat[i].textured) {
            lightingToggle = " #LIGHTING";
        }

        mat[i].setName(aiName.data + lightingToggle);

        //std::cout << "Texture (absolute) " << aiPath.data << std::endl;
        //std::cout << "Texture (relative) " << get_path(file) + aiPath.data << std::endl;
    }
}

static inline bool has_texture_type(const std::string &path) {
    for (u8 i = 0; i < FORMATS; i++) {
        if (path.find(format[i]) != std::string::npos) {
            return true;
        }
    }
    return false;
}

static void write_textures(const std::string &fileOut, Material *mat, const aiScene* scene, bool level) {
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

            if (has_texture_type(mat[i].getPath())) {
                if (file_exists(mat[i].getPath())) {
                    remove_file(fileOut + "/" + get_filename(mat[i].getPath()));
                }

                copy_file(mat[i].getPath(), fileOut + "/" + get_filename(mat[i].getPath()));
            } else {
                if (file_exists(mat[i].getPath().substr(0, mat[i].getPath().length() - 4) + ".rgba16.png")) {
                    remove_file(fileOut + "/" + get_filename(mat[i].getPath().substr(0, mat[i].getPath().length() - 4) + ".rgba16.png"));
                }

                copy_file(mat[i].getPath(), fileOut + "/" + get_filename(mat[i].getPath().substr(0, mat[i].getPath().length() - 4) + ".rgba16.png"));
            }
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

static inline void set_layers_amt() {
    for (u8 i = 0; i < 8; i++) {
        if (setLayer[i]) {
            layers++;
        }
    }

    if (layers == 0) { /* default to layer 1 if no layer is specified */
        setLayer[0] = true;
        layers = 1;
    }
}

static inline void set_layers(DisplayList *dl) {
    u8 index = 0;
    for (u8 i = 0; i < 8; i++) {
        if (setLayer[i]) {
            dl[index++].setLayer(i);
        }
    }
}

/* New DL writer */
static inline void write_display_list_obj(const std::string &fileOut, VertexBuffer* vBuf, DisplayList* dl, Material* mat) {
    for (u8 i = 0; i < layers; i++) {
        vBuf[i].vtxCount = 0;
        dl[i].writeDisplayList(fileOut, vBuf, vBuffers, mat);
    }
}

/** Main function for the F3D build process. */
void f3d_main(const std::string &file, const std::string &fileOut, s16 scale, u8 microcode, bool level, bool uvFlip) {
    Assimp::Importer importer;

    /* We don't use ASSIMP's built in tristripping because of the vertex buffer. */
    const aiScene* scene = importer.ReadFile(file, aiProcess_ValidateDataStructure | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_GenUVCoords);

    reset_file(fileOut + "/model.s");
    inspect_vtx(scene->mRootNode, scene);

    vBuffers = vert / microcode;

    if (vert % microcode > 0) { /* is there a trailing vbuffer? */
        vBuffers++;
    }

    info_message("Vertices " + std::to_string(vert));
    info_message("Triangles " + std::to_string(vert / 3));

    VertexBuffer vBuf[vBuffers];
    cycle_vbuffers(vBuf, BUFFER, microcode);

    /* Materials */
    Material mat[scene->mNumMaterials];
    configure_materials(file, fileOut, mat, scene);
    write_textures(fileOut, mat, scene, level);

    setup_vtx(scene->mRootNode, scene, scale, vBuf, file, uvFlip, mat);
    cycle_vbuffers(vBuf, OPTIMIZE, 0);
    write_vtx(fileOut, "", vBuf);

    cycle_vbuffers(vBuf, RESET, 0);
    set_layers_amt();
    DisplayList dl[layers];
    set_layers(dl);
    write_display_list_obj(fileOut, vBuf, dl, mat);
}
