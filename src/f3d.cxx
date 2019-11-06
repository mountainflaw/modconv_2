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

#include "modconv.hxx"
#include "buffer.hxx"
#include "material.hxx"
#include "displaylist.hxx"

#include "f3d.hxx"

/* global variabes */

u32 vert     = 0,
    vert2    = 0;
u16 vBuffers = 0,
    vBuffer  = 0,
    bone     = 0;
u8  layers   = 0;
bool setLayer[8] = { false };

static CullRegion cullBox;

u8 diffuse[6] = {0xFF, 0xFF, 0xFF, 0x28, 0x28, 0x28}, ambient[3] = {0x66, 0x66, 0x66};

u32 geometryState = 0;

bool fog = false;
u16 fogSettings[6]; /* rgba near far */

const std::string format[FORMATS] = { ".rgba16.png", ".rgba32.png", ".ci4.png", ".ci8.png", ".ia4.png", ".ia8.png", ".i4.png", ".i8.png" };

const std::string dlTypes[8] = {
    "force",
    "opaque",
    "opaque_decal",
    "opaque_inter",
    "alpha",
    "transparent",
    "transparent_decal",
    "transparent_inter"
};

const std::string layerTypes[8] = {
    "LAYER_FORCE",
    "LAYER_OPAQUE",
    "LAYER_OPAQUE_DECAL",
    "LAYER_OPAQUE_INTER",
    "LAYER_ALPHA",
    "LAYER_TRANSPARENT",
    "LAYER_TRANSPARENT_DECAL",
    "LAYER_TRANSPARENT_INTER"
};

INLINE std::string dl_command(const std::string &cmd, const std::string &arg) {
    return "    " + cmd + "(" + arg + "),";
}

INLINE std::string dl_command(const std::string &cmd) {
    return "    " + cmd + "(),";
}

INLINE std::string dl_command_ref(const std::string &cmd, const std::string &arg) {
    return "    " + cmd + "(&" + arg + "),";
}

void inspect_vtx(aiNode* node, const aiScene* scene) {
    for (u16 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        if (mesh->HasPositions() && mesh->HasFaces()) {
            for (u32 j = 0; j < mesh->mNumFaces; j++) {
                if (mesh->mFaces[j].mNumIndices == 3) {
                    vert += 3;
                }
            }
        }
    }

    for (u16 i = 0; i < node->mNumChildren; i++) {
        inspect_vtx(node->mChildren[i], scene);
    }
}

namespace uvutil {
    enum UvUtilFirstLast { FIRST, LAST };

    /* Min */

    static INLINE u8 sort_u_last(s32 uv[3][2]) {
        if (uv[1][AXIS_X] < uv[0][AXIS_X]) {
            return 1;
        }

        if (uv[2][AXIS_X] < uv[0][AXIS_X]) {
            return 2;
        }

        return 0;
    }

    static INLINE u8 sort_v_last(s32 uv[3][2]) {
        if (uv[1][AXIS_Y] < uv[0][AXIS_Y]) {
            return 1;
        }

        if (uv[2][AXIS_Y] < uv[0][AXIS_Y]) {
            return 2;
        }

        return 0;
    }

    /* Max */

    static INLINE u8 sort_u_first(s32 uv[3][2]) {
        if (uv[1][AXIS_X] > uv[0][AXIS_X]) {
            return 1;
        }

        if (uv[2][AXIS_X] > uv[0][AXIS_X]) {
            return 2;
        }

        return 0;
    }

    static INLINE u8 sort_v_first(s32 uv[3][2]) {
        if (uv[1][AXIS_Y] > uv[0][AXIS_Y]) {
            return 1;
        }

        if (uv[2][AXIS_Y] > uv[0][AXIS_Y]) {
            return 2;
        }

        return 0;
    }

    /**
     * Fixes UVs above s16 limits.
     * Credits to pilzinsel's for originally implementing
     * this in Super Mario 64 ROM Manager.
     */

    static void reset_uv(s32 uv[3][2], u16 w, u16 h, bool nearest) {
        u16 jump = w * 0x40;
        u8 sortUv[2][2] = {{sort_u_last(uv), sort_v_last(uv)}, {sort_u_first(uv), sort_v_first(uv)}};

        while (uv[sortUv[LAST][AXIS_X]][AXIS_X] > 32767) {
            uv[0][AXIS_X] -= jump;
            uv[1][AXIS_X] -= jump;
            uv[2][AXIS_X] -= jump;

        }

        while (uv[sortUv[FIRST][AXIS_X]][AXIS_X] < -32768) {
            uv[0][AXIS_X] += jump;
            uv[1][AXIS_X] += jump;
            uv[2][AXIS_X] += jump;
        }

        jump = h * 0x40;

        while (uv[sortUv[LAST][AXIS_Y]][AXIS_Y] > 32767) {
            uv[0][AXIS_Y] -= jump;
            uv[1][AXIS_Y] -= jump;
            uv[2][AXIS_Y] -= jump;
        }

        while (uv[sortUv[FIRST][AXIS_Y]][AXIS_Y] < -32768) {
            uv[0][AXIS_Y] += jump;
            uv[1][AXIS_Y] += jump;
            uv[2][AXIS_Y] += jump;
        }

        if (!gUvFlip) {
            uv[0][AXIS_Y] *= -1;
            uv[1][AXIS_Y] *= -1;
            uv[2][AXIS_Y] *= -1;
        }

        if (!nearest) {
            uv[0][AXIS_X] -= 32;
            uv[1][AXIS_X] -= 32;
            uv[2][AXIS_X] -= 32;

            uv[0][AXIS_Y] -= 32;
            uv[1][AXIS_Y] -= 32;
            uv[2][AXIS_Y] -= 32;
        }
    }
}

/** Add vertices to vertex buffers. */
void setup_vtx(aiNode *node, const aiScene* scene, s16 scale,
        VertexBuffer* vBuf, const std::string &file, Material* mat) {
    for (u16 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        /* we go by faces instead of verts so we don't accidentally add what we don't need */
        for (u32 j = 0; j < mesh->mNumFaces; j++) {
            s32 uv[3][2] = {0x00};

            /* Calculate UVs per triangle, in case we have to correct them. */
            if (scene->HasMaterials() && mesh->HasTextureCoords(0) && mat[mesh->mMaterialIndex].textured) {
                uv[0][AXIS_X] = mesh->mTextureCoords[0][mesh->mFaces[j].mIndices[0]].x * 32 * mat[mesh->mMaterialIndex].getDimension(AXIS_X);
                uv[1][AXIS_X] = mesh->mTextureCoords[0][mesh->mFaces[j].mIndices[1]].x * 32 * mat[mesh->mMaterialIndex].getDimension(AXIS_X);
                uv[2][AXIS_X] = mesh->mTextureCoords[0][mesh->mFaces[j].mIndices[2]].x * 32 * mat[mesh->mMaterialIndex].getDimension(AXIS_X);

                uv[0][AXIS_Y] = mesh->mTextureCoords[0][mesh->mFaces[j].mIndices[0]].y * 32 * mat[mesh->mMaterialIndex].getDimension(AXIS_Y);
                uv[1][AXIS_Y] = mesh->mTextureCoords[0][mesh->mFaces[j].mIndices[1]].y * 32 * mat[mesh->mMaterialIndex].getDimension(AXIS_Y);
                uv[2][AXIS_Y] = mesh->mTextureCoords[0][mesh->mFaces[j].mIndices[2]].y * 32 * mat[mesh->mMaterialIndex].getDimension(AXIS_Y);

                uvutil::reset_uv(uv, mat[mesh->mMaterialIndex].getDimension(AXIS_X), mat[mesh->mMaterialIndex].getDimension(AXIS_Y), mat[mesh->mMaterialIndex].isNearest());
            }

            for (u8 k = 0; k < 3; k++) {
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

                if (mesh->HasPositions() && mesh->HasFaces() && mesh->mFaces[j].mNumIndices == 3) {
                    u32 currVtx = mesh->mFaces[j].mIndices[k];

                    s16 pos[3];

                    pos[AXIS_X] = (s16)(((mesh->mVertices[currVtx].x) * scale) * scaling_hack());
                    pos[AXIS_Y] = (s16)(((mesh->mVertices[currVtx].y) * scale) * scaling_hack());
                    pos[AXIS_Z] = (s16)(((mesh->mVertices[currVtx].z) * scale) * scaling_hack());

                    if (pos[AXIS_X] > cullBox.x0) { /* generate bounding box */
                        cullBox.x0 = pos[AXIS_X];
                    } else if (pos[AXIS_X] < cullBox.x1) {
                        cullBox.x1 = pos[AXIS_X];
                    } else if (pos[AXIS_Z] > cullBox.z0) {
                        cullBox.z0 = pos[AXIS_Z];
                    } else if (pos[AXIS_Z] < cullBox.z1) {
                        cullBox.z1 = pos[AXIS_Z];
                    } else if (pos[AXIS_Y] > cullBox.y0) {
                        cullBox.y0 = pos[AXIS_Y];
                    } else if (pos[AXIS_Y] < cullBox.y1) {
                        cullBox.y1 = pos[AXIS_Y];
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
                                || nameStr.find("#NORMCOLOR") != std::string::npos || !mat[mesh->mMaterialIndex].textured)) {

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
                        const std::string layerTags[8] = { "#LAYER_0", "#LAYER_1", "#LAYER_2", "#LAYER_3", "#LAYER_4", "#LAYER_5", "#LAYER_6", "#LAYER_7" };

                        for (u8 l = 0; l < 8; l++) {
                            if (nameStr.find(layerTags[l]) != std::string::npos) {
                                layer = l;
                                setLayer[l] = true;
                            }
                        }

                        if (layer == 1) { /* default layer if none set */
                            setLayer[1] = true;
                        }
                    } else { /* ditto, but if there are no materials */
                        setLayer[1] = true;
                    }

                    vBuf[vBuffer].addVtx(pos[AXIS_X], pos[AXIS_Y], pos[AXIS_Z],
                            (s16)uv[k][AXIS_X], (s16)uv[k][AXIS_Y],
                            rgba[C_RED], rgba[C_GRN], rgba[C_BLU], rgba[C_APH], mesh->mMaterialIndex, layer);
                    vert2++;
                }
            }
        }
    }
    for (u16 i = 0; i < node->mNumChildren; i++) {
        setup_vtx(node->mChildren[i], scene, scale, vBuf, file, mat);
    }
}

/** Function for common vbuffer operations (reset counter, run the optimizer, etc). */
void cycle_vbuffers(VertexBuffer *vBuf, u8 mode, u8 microcode) {
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

INLINE std::string hex_string(const u8 hex) {
    std::stringstream s;
    s << std::hex << (u16)hex;

    if (hex < 0x10) {
        return "0x0" + s.str();
    }

    return "0x" + s.str();
}

static void write_vtx(const std::string fileOut, const std::string &path, VertexBuffer *vBuf) {
    std::fstream vtxOut;
    vtxOut.open(fileOut + "/model.inc.c", std::ofstream::out | std::ofstream::app);

    for (u16 i = 0; i < vBuffers; i++) {
        vtxOut << std::endl << labelize("Vtx ") << get_filename(fileOut) << "_vertex_" << i << "[" << (u16)vBuf[i].loadSize << "] = { /* " << (u16)vBuf[i].loadSize << " vertices out of " << (u16)vBuf[i].bufferSize << " */" << std::endl;

        if (glabel) {
            extern_data(fileOut, "extern Vtx " + get_filename(fileOut) + "_vertex_" + std::to_string(i) + "[" + std::to_string((u16)vBuf[i].loadSize) + "];");
        }

        for (u16 j = 0; j < vBuf[i].bufferSize; j++) {
            Vertex vtx = vBuf[i].getVtx();
            if (!vtx.useless) {
                vtxOut << "    {"
                       << "{{ " << std::right << std::setw(6) << vtx.pos[AXIS_X] << ", " << std::right << std::setw(6)
                       << vtx.pos[AXIS_Y] << ", " << std::right << std::setw(6)
                       << vtx.pos[AXIS_Z] << "}, 0x00, {" << std::right << std::setw(7)
                       << vtx.st[AXIS_X]  << ", " << std::right << std::setw(6)
                       << vtx.st[AXIS_Y]  << "}, "
                       << "{" << hex_string(vtx.col[C_RED])  << ", " << std::right << std::setw(4)
                       << hex_string(vtx.col[C_GRN])  << ", " << std::right << std::setw(4)
                       << hex_string(vtx.col[C_BLU])  << ", " << std::right << std::setw(4)
                       << hex_string(vtx.col[C_APH])  << "}}}," << std::endl;
            }
        }
        vtxOut << "};" << std::endl;
    }

    if (gCullDlist) {
        vtxOut << labelize(get_filename(fileOut) + "_culling_vertex") << std::endl;
    }
}

void configure_materials(const std::string &file, const std::string &fileOut, Material* mat, const aiScene* scene) {
    aiColor4D aiDiffuse;
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

        if (scene->mMaterials[i]->Get(AI_MATKEY_COLOR_DIFFUSE, aiDiffuse) == AI_SUCCESS) {
            mat[i].diffuse[C_RED] = aiDiffuse.r * 0xFF;
            mat[i].diffuse[C_GRN] = aiDiffuse.g * 0xFF;
            mat[i].diffuse[C_BLU] = aiDiffuse.b * 0xFF;
        } else { /* no diffuse colors */
            mat[i].diffuse[C_RED] = 0xFF;
            mat[i].diffuse[C_GRN] = 0xFF;
            mat[i].diffuse[C_BLU] = 0xFF;
        }

        /* Untextured models are always shaded. */
        std::string lightingToggle = "";
        if (!mat[i].textured) {
            lightingToggle = " #LIGHTING";
        }

        mat[i].setName(aiName.data + lightingToggle);
        mat[i].setIndex(i);
    }
}

static INLINE bool has_texture_type(const std::string &path) {
    for (u8 i = 0; i < FORMATS; i++) {
        if (path.find(format[i]) != std::string::npos) {
            return true;
        }
    }
    return false;
}

static INLINE std::string get_tex_incbin(const std::string &incbin) {
    std::string incbinFile = get_filename(incbin);
    if (has_texture_type(incbin)) {
        return incbinFile.substr(0, incbinFile.length() - 4);
    } else {
        return incbinFile.substr(0, incbinFile.length() - 4) + ".rgba16";
    }
}

static void write_textures(const std::string &fileOut, Material *mat, const aiScene* scene, bool level) {
    std::fstream texOut;
    if (level) {
        reset_file(fileOut + "/texture.inc.c");
        texOut.open(fileOut + "/texture.inc.c", std::ofstream::out | std::ofstream::app);
    } else { /* generating an actor */
        texOut.open(fileOut + "/model.inc.c", std::ofstream::out | std::ofstream::app);
    }


    /* Phase 1: Copy lights */

    texOut << std::endl << labelize("Ambient ") << get_filename(fileOut) << "_ambient_light = {" << std::endl << "{{" << hex_string(ambient[0]) << ", " << hex_string(ambient[1]) << ", " + hex_string(ambient[2]) << "}, 0x00, {" << hex_string(ambient[0]) + ", " << hex_string(ambient[1]) << ", " + hex_string(ambient[2]) << "}, 0x00}" << std::endl << "};" << std::endl
           << labelize("Light ") << get_filename(fileOut) << "_diffuse_light = {" << std::endl << "{{" << hex_string(diffuse[0]) + ", " + hex_string(diffuse[1]) + ", " + hex_string(diffuse[2]) + "}, 0x00, {" + hex_string(diffuse[0]) + ", " + hex_string(diffuse[1]) + ", " + hex_string(diffuse[2]) + "}, 0x00, {" << hex_string(diffuse[3]) + ", " + hex_string(diffuse[4]) + ", " + hex_string(diffuse[5]) + "}, 0x00}" << std::endl << "};" << std::endl;

    /* Phase 2 - Find redundant textures */

    for (u16 i = 0; i < scene->mNumMaterials; i++) {
        for (u16 j = 0; j < scene->mNumMaterials; j++) {
            if (mat[i].getPath().compare(mat[j].getPath()) == 0 && j > i) {
                mat[j].useless = true;
                mat[j].index = i;
            }
        }
    }

    /* Phase 2: Write and copy textures */

    for (u16 i = 0; i < scene->mNumMaterials; i++) {
        std::string texturePath;

        if (has_texture_type(mat[i].getPath())) {
            texturePath = get_filename(mat[i].getPath());
        } else {
            texturePath = get_filename(mat[i].getPath().substr(0, mat[i].getPath().length() - 4) + ".rgba16.png");
        }

        if (!mat[i].useless && mat[i].textured) {
            std::string exportType;

            if (level) {
                exportType = "levels";
            } else {
                exportType = "actors";
            }

            texOut << std::endl;
            texOut << "ALIGNED8 " << labelize("u8 " + fileOut + "_texture_" + std::to_string(mat[i].index)) << "[] = {" << std::endl;
            texOut << "#include " << R"(")" << exportType << "/" << get_filename(fileOut) << "/" << get_tex_incbin(mat[i].getPath()) << R"(.inc.c")" << std::endl;
            texOut << "};" << std::endl;

            if (mat[i].getFileNameNoExtension().find("ci4") != std::string::npos || mat[i].getFileNameNoExtension().find("ci8") != std::string::npos) { /* CI palette */
                texOut << std::endl << "ALIGNED8" << labelize("u8 " + fileOut + "_palette_" + std::to_string(mat[i].index)) << "[] = {" << std::endl;
                texOut << "#include " << R"(")" << exportType << "/" << get_filename(fileOut) << "/" << mat[i].getFileNameNoExtension() << R"(.pal.inc.c")" << std::endl;
                texOut << "};" << std::endl;
            }

            if (file_exists(fileOut + "/" + texturePath)) {
                remove_file(fileOut + "/" + texturePath);
            }

            copy_file(mat[i].getPath(), fileOut + "/" + texturePath);
        }
    }
}

/** Write display list commands to file. */
static INLINE void set_layers_amt() {
    for (u8 i = 0; i < 8; i++) {
        if (setLayer[i]) {
            layers++;
        }
    }
}

static INLINE void set_layers(DisplayList *dl) {
    u8 index = 0;
    for (u8 i = 0; i < 8; i++) {
        if (setLayer[i]) {
            dl[index++].setLayer(i);
        }
    }
}

/* New DL writer */
static INLINE void write_display_list_obj(const std::string &fileOut, VertexBuffer* vBuf, DisplayList* dl, Material* mat) {
    for (u8 i = 0; i < layers; i++) {
        vBuf[i].vtxCount = 0;
        dl[i].writeDisplayList(fileOut, vBuf, vBuffers, mat);
        geometryState = 0; /* Reset geometry state */
    }
}

static INLINE std::string dl_tab(bool level) {
    if (level) {
        return "                    ";
    }
    return "            ";
}
static void write_geometry_layout(const std::string &fileOut, bool level) {
    std::fstream geoOut;
    reset_file(fileOut + "/geo.inc.c");
    geoOut.open(fileOut + "/geo.inc.c", std::ofstream::out | std::ofstream::app);


    geoOut << std::endl << "const GeoLayout " << get_filename(fileOut) << "_geo[] = {" << std::endl;

    if (level) {
        geoOut << "    GEO_NODE_SCREEN_AREA(10, SCREEN_WIDTH/2, SCREEN_HEIGHT/2, SCREEN_WIDTH/2, SCREEN_HEIGHT/2)," << std::endl
               << "    GEO_OPEN_NODE()," << std::endl
               << "        GEO_ZBUFFER(0)," << std::endl
               << "        GEO_OPEN_NODE()," << std::endl
               << "            GEO_NODE_ORTHO(100)," << std::endl
               << "            GEO_OPEN_NODE()," << std::endl
               << "                GEO_BACKGROUND(BACKGROUND_OCEAN_SKY, geo_skybox_main)," << std::endl
               << "            GEO_CLOSE_NODE()," << std::endl
               << "        GEO_CLOSE_NODE()," << std::endl
               << "        GEO_ZBUFFER(1)," << std::endl
               << "        GEO_OPEN_NODE()," << std::endl
               << "            GEO_CAMERA_FRUSTUM_WITH_FUNC(45, 100, 30000, geo_camera_fov)," << std::endl
               << "            GEO_OPEN_NODE()," << std::endl
               << "                GEO_CAMERA(1, 0, 2000, 6000, 3072, 0, -4608, geo_camera_preset_and_pos)," << std::endl
               << "                GEO_OPEN_NODE()," << std::endl;
    } else { /* actors */
        geoOut << "    GEO_SHADOW(SHADOW_CIRCLE_4_VERTS, 0xC8, 60)," << std::endl
               << "    GEO_OPEN_NODE()," << std::endl
               << "        GEO_SCALE(0x00, 16384)," << std::endl
               << "        GEO_OPEN_NODE()," << std::endl;
    }

    for (u8 i = 0; i < 8; i++) { /* Insert display lists */
        if (setLayer[i]) {
            geoOut << dl_tab(level) << "GEO_DISPLAY_LIST(" << layerTypes[i] << ", " << get_filename(fileOut) << "_dl_" << dlTypes[i] << ")," << std::endl;
        }
    }

    if (level) {
        geoOut << "                    GEO_RENDER_OBJ()," << std::endl
               << "                    GEO_ASM(0, geo_enfvx_main)," << std::endl
               << "                GEO_CLOSE_NODE()," << std::endl
               << "            GEO_CLOSE_NODE()," << std::endl
               << "        GEO_CLOSE_NODE()," << std::endl
               << "        GEO_ZBUFFER(0)," << std::endl
               << "        GEO_OPEN_NODE()," << std::endl
               << "            GEO_ASM(0, Geo18_802CD1E8)," << std::endl;
        geoOut << "        GEO_CLOSE_NODE()," << std::endl
               << "    GEO_CLOSE_NODE()," << std::endl
               << "    GEO_END()" << std::endl
               << "};" << std::endl;
    } else {
        geoOut << "        GEO_CLOSE_NODE()," << std::endl
               << "    GEO_CLOSE_NODE()," << std::endl
               << "    GEO_END()" << std::endl
               << "};" << std::endl;
    }
}

/** Main function for the F3D build process. */
void f3d_main(const std::string &file, const std::string &fileOut, s16 scale, u8 microcode, bool level) {
    Assimp::Importer importer;

    /* We don't use ASSIMP's built in tristripping because of the vertex buffer. */
    const aiScene* scene = importer.ReadFile(file, aiProcess_ValidateDataStructure | aiProcess_Triangulate | aiProcess_PreTransformVertices);

    reset_file(fileOut + "/model.inc.c");

    inspect_vtx(scene->mRootNode, scene);

    vBuffers = vert / microcode;

    if (vert % microcode > 0) { /* is there a trailing vbuffer? */
        vBuffers++;
    }

    info_message("Vertices " + std::to_string(vert));
    info_message("Triangles " + std::to_string(vert / 3));


    if (gCullDlist &&gCullDlist &&  vert < 3) {
        error_message("Cannot use culldl on single triangle models!");
    }

    VertexBuffer vBuf[vBuffers];
    cycle_vbuffers(vBuf, BUFFER, microcode);

    /* Materials */
    Material mat[scene->mNumMaterials];
    configure_materials(file, fileOut, mat, scene);
    write_textures(fileOut, mat, scene, level);

    setup_vtx(scene->mRootNode, scene, scale, vBuf, file, mat);
    cycle_vbuffers(vBuf, OPTIMIZE, 0);
    write_vtx(fileOut, "", vBuf);

    cycle_vbuffers(vBuf, RESET, 0);
    set_layers_amt();
    DisplayList dl[layers];
    set_layers(dl);
    write_display_list_obj(fileOut, vBuf, dl, mat);
    write_geometry_layout(fileOut, level);

    for (u8 i = 0; i < 8; i++) { /* Insert display lists in header */
        if (setLayer[i]) {
            if (glabel) {
                extern_data(fileOut, "extern Gfx " + get_filename(fileOut) + "_dl_" + dlTypes[i] + "[];\n");
            } else {
                extern_data(fileOut, "extern const Gfx " + get_filename(fileOut) + "_dl_" + dlTypes[i] + "[];\n");
            }
        }
    }

    if (glabel) {
        extern_data(fileOut, "extern GeoLayout " + get_filename(fileOut) + "_geo[]\n");
    } else {
        extern_data(fileOut, "extern const GeoLayout " + get_filename(fileOut) + "_geo[]\n");
    }
}
