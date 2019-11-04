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

/* This file handles creating collision data out of a model imported with ASSIMP. */
#include "modconv.hxx"

/* Enums */

enum { WATERBOX_WATER, WATERBOX_GAS=50 };

/* Structs */

struct CollisionVtx {
    s16 pos[3];
    u32 list;
    u16 material;
    bool useless;
};

struct CollisionMat {
    bool isSpecial;
    u16 tri;
    std::string surf;
    std::string special;
};

struct CollisionWaterBox {
    u8 type;
    s16 x1;
    s16 z1;
    s16 x2;
    s16 z2;
    s16 y;
    u16 material;
};

/* Statics */

static u32 vertex = 0, internalVtx = 0, writeSize = 0;
static std::vector<CollisionWaterBox> waterBox;

/**
 * Configure material and surface properties.
 */

static void configure_materials(const aiScene* scene, CollisionMat* mat) {
    aiString aiName;
    u16 pos[2] = { 0 };
    for (u16 i = 0; i < scene->mNumMaterials; i++) {
        scene->mMaterials[i]->Get(AI_MATKEY_NAME, aiName);
        mat[i].tri = 0;

        std::string nameStr = aiName.data;

        enum SurfaceString { STARTPOS, ENDPOS };

        /*
         * Find surface setting:
         * This searches for the first instance of !,
         * which will mark the beginning of the the surface
         * set, the ending is either the end of the string
         * or until the first instance of a space.
         */

        /* Special triangles */

        if (nameStr.find("~") != std::string::npos) {
            mat[i].isSpecial = true;
            pos[STARTPOS] = nameStr.find("~") + 1;
            for (u16 j = pos[STARTPOS]; j < nameStr.length(); j++) {
                if (nameStr[j] == ' ') {
                    break;
                }
                mat[i].special = nameStr.substr(pos[STARTPOS], (j - pos[STARTPOS]) + 1);
            }
            info_message("Special: " + nameStr + " -> " + mat[i].special);
        } else {
            mat[i].isSpecial = false;
        }

        pos[STARTPOS] = 0;

        /* Surface ID */

        if (nameStr.find("!") != std::string::npos) {
            pos[STARTPOS] = nameStr.find("!") + 1;
            for (u16 j = pos[STARTPOS]; j < nameStr.length(); j++) {
                if (nameStr[j] == ' ') {
                    break;
                }
                mat[i].surf = nameStr.substr(pos[STARTPOS], (j - pos[STARTPOS]) + 1);
            }
        } else {
            mat[i].surf = "SURFACE_DEFAULT";
        }
        info_message("Material to surface: " + nameStr + " -> " + mat[i].surf);
    }
}

/**
 * Adds material entry to waterbox vector.
 */

static INLINE void add_waterbox_material(u8 type, u8 m) {
    for (u8 i = 0; i < waterBox.size(); i++) {
        if (m == waterBox[i].material) { /* Don't add more than what we need */
            return;
        }
    }
    waterBox.push_back({type, 0, 0, 0, 0, 0, m});
}

/**
 * Calculate amount of vertices, triangles per material, and waterboxes.
 */

static void inspect_vtx(const aiNode* node, const aiScene* scene, CollisionMat* mat) {
    std::string nameStr = "";

    for (u16 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        if (scene->HasMaterials()) {
            aiString aiName;
            scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_NAME, aiName);
            nameStr = aiName.data;
        }

        for (u32 j = 0; j < mesh->mNumFaces; j++) {
            if (mesh->HasPositions() && mesh->HasFaces() && mesh->mFaces[j].mNumIndices == 3
                    && nameStr.find("@WATER") == std::string::npos && nameStr.find("@GAS") == std::string::npos) { /* Don't allow lines and points to be added. */
                vertex += 3;
            } else if (nameStr.find("@WATER") != std::string::npos && waterBox.size() < 255) {
                add_waterbox_material(WATERBOX_WATER, mesh->mMaterialIndex);
            } else if (nameStr.find("@GAS") != std::string::npos && waterBox.size() < 255) {
                add_waterbox_material(WATERBOX_GAS, mesh->mMaterialIndex);
            }

        }
        mat[mesh->mMaterialIndex].tri += mesh->mNumFaces;
    }

    for (u16 i = 0; i < node->mNumChildren; i++) {
        inspect_vtx(node->mChildren[i], scene, mat);
    }
}

/**
 * Add non waterbox vertices to vertex list.
 */

static void setup_vtx(const std::string &file, aiNode* node, const aiScene* scene, CollisionVtx* vtx, const s16 scale) {
    std::string nameStr = "";

    for (u16 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        if (scene->HasMaterials()) {
            aiString aiName;
            scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_NAME, aiName);
            nameStr = aiName.data;
        }

        /* We go by faces, so we don't add loose geometry to our output. */
        for (u32 j = 0; j < mesh->mNumFaces; j++) {
            for (u8 k = 0; k < 3; k++) {
                if (mesh->HasPositions() && mesh->HasFaces() && mesh->mFaces[j].mNumIndices == 3 && nameStr.find("@WATER") == std::string::npos && nameStr.find("@GAS") == std::string::npos) { /* Prevent potential segfault */
                    u32 currVtx = mesh->mFaces[j].mIndices[k];
                    vtx[internalVtx].pos[AXIS_X] = (s16)(((mesh->mVertices[currVtx].x) * scale) * scaling_hack());
                    vtx[internalVtx].pos[AXIS_Y] = (s16)(((mesh->mVertices[currVtx].y) * scale) * scaling_hack());
                    vtx[internalVtx].pos[AXIS_Z] = (s16)(((mesh->mVertices[currVtx].z) * scale) * scaling_hack());

                    vtx[internalVtx].useless = false;
                    vtx[internalVtx].list = internalVtx;
                    vtx[internalVtx].material = mesh->mMaterialIndex;
                    internalVtx++;
                }
            }
        }
    }

    for (u16 i = 0; i < node->mNumChildren; i++) {
        setup_vtx(file, node->mChildren[i], scene, vtx, scale);
    }
}

/**
 * Compare two vertices with leniency factor.
 */

static INLINE bool cprVtx(const CollisionVtx* vtx, const u32 i, const u32 j) {
    return abs(vtx[i].pos[AXIS_X] - vtx[j].pos[AXIS_X]) <= leniencyFactor &&
           abs(vtx[i].pos[AXIS_Y] - vtx[j].pos[AXIS_Y]) <= leniencyFactor &&
           abs(vtx[i].pos[AXIS_Z] - vtx[j].pos[AXIS_Z]) <= leniencyFactor;

}

/**
 * Remove redundant vertices.
 */

static void clean_vtx(CollisionVtx* vtx) {
    /* Stage 1 - Mark redundant vertices */
    for (u32 i = 0; i < vertex; i++) {
        for (u32 j = 0; j < vertex; j++) {
            if (cprVtx(vtx, i, j) && j > i && !vtx[j].useless) {
                vtx[j].useless = true;
                vtx[j].list = i;
            }
        }
    }

    /* Stage 2 - Give verts that will be written their correct index. */
    for (u32 i = 0; i < vertex; i++) {
        if (!vtx[i].useless) {
            vtx[i].list = writeSize;
            writeSize++;
        }
    }
}

/**
 * Write optimized vertex data.
 */

static void write_vtx(const std::string &fileOut, const CollisionVtx* vtx) {
    std::fstream colOut;
    colOut.open(fileOut + "/collision.inc.c", std::iostream::out | std::iostream::app);
    if (writeSize == 0) {
        writeSize = vertex; /* handle case with no stripping */
    }
    colOut << "    COL_VERTEX_INIT(" << writeSize << ")," << std::endl;
    for (u32 i = 0; i < vertex; i++) {
        if (vtx[i].useless == false) {
            colOut << "    COL_VERTEX(" << std::right << std::setw(6)
                                   << vtx[i].pos[AXIS_X] << ", " << std::right << std::setw(6)
                                   << vtx[i].pos[AXIS_Y] << ", " << std::right << std::setw(6)
                                   << vtx[i].pos[AXIS_Z] << ")," << std::endl;

        }
    }
}

/**
 * Retrieves a collision vertex from optimized list.
 */

static INLINE u32 get_vtx_index(const CollisionVtx* vtx, const u32 pos) {
    if (vtx[pos].useless) { /* optimized out */
        return vtx[vtx[pos].list].list;
    } else { /* original vertex */
        return vtx[pos].list;
    }
}

/**
 * Compares two waterbox materials.
 */

static INLINE bool waterbox_sort(const CollisionWaterBox &a, const CollisionWaterBox &b) {
    return a.y > b.y;
}

/**
 * Creates waterbox bounding box.
 */

static void configure_waterbox(const aiScene* scene, const aiNode* node, s16 scale, CollisionWaterBox &waterBox) {
    for (u16 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        if (waterBox.material == mesh->mMaterialIndex) {
            for (u32 j = 0; j < mesh->mNumFaces; j++) {
                for (u8 k = 0; k < 3; k++) {
                    if ((s16)((mesh->mVertices[mesh->mFaces[j].mIndices[k]].x * scale) * scaling_hack()) > waterBox.x1) {
                        waterBox.x1 = (s16)((mesh->mVertices[mesh->mFaces[j].mIndices[k]].x * scale) * scaling_hack());
                    }

                    if ((s16)((mesh->mVertices[mesh->mFaces[j].mIndices[k]].x * scale) * scaling_hack()) < waterBox.x2) {
                        waterBox.x2 = (s16)((mesh->mVertices[mesh->mFaces[j].mIndices[k]].x * scale) * scaling_hack());
                    }

                    if ((s16)((mesh->mVertices[mesh->mFaces[j].mIndices[k]].z * scale) * scaling_hack()) > waterBox.z1) {
                        waterBox.z1 = (s16)((mesh->mVertices[mesh->mFaces[j].mIndices[k]].z * scale) * scaling_hack());
                    }

                    if ((s16)((mesh->mVertices[mesh->mFaces[j].mIndices[k]].z * scale) * scaling_hack()) < waterBox.z2) {
                        waterBox.z2 = (s16)((mesh->mVertices[mesh->mFaces[j].mIndices[k]].z * scale) * scaling_hack());
                    }

                    if ((s16)((mesh->mVertices[mesh->mFaces[j].mIndices[k]].y * scale) * scaling_hack()) > waterBox.y) {
                        waterBox.y = (s16)((mesh->mVertices[mesh->mFaces[j].mIndices[k]].y * scale) * scaling_hack());
                    }
                }
            }
        }
    }

    for (u16 i = 0; i < node->mNumChildren; i++) {
        configure_waterbox(scene, node->mChildren[i], scale, waterBox);
    }
}

/**
 * Write collision data to disk.
 */

static void write_tri(const std::string &fileOut, const CollisionVtx* vtx, const CollisionMat* mat) {
    u16 currSurf = 0;

    std::fstream colOut;
    colOut.open(fileOut + "/collision.inc.c", std::iostream::out | std::iostream::app);

    for (u32 i = 0; i < vertex; i += 3) {
        if (vtx[i].material != currSurf || i == 0) {
            colOut << std::endl;
            currSurf = vtx[i].material;
            colOut << "    COL_TRI_INIT(" << mat[vtx[i].material].surf << ", " << mat[vtx[i].material].tri << ")," << std::endl;
        }

        if (mat[vtx[i].material].isSpecial) {
            colOut << "    COL_TRI_SPECIAL(" << std::right << std::setw(6)
                                       << get_vtx_index(vtx, i) << ", " << std::right << std::setw(6)
                                       << get_vtx_index(vtx, i + 1) << ", " << std::right << std::setw(6)
                                       << get_vtx_index(vtx, i + 2) << ", " << std::right << std::setw(6)
                                       << mat[vtx[i].material].special << ")," << std::endl;
        } else {
            colOut << "    COL_TRI(" << std::right << std::setw(6)
                                << get_vtx_index(vtx, i) << ", " << std::right << std::setw(6)
                                << get_vtx_index(vtx, i + 1) << ", " << std::right << std::setw(6)
                                << get_vtx_index(vtx, i + 2) << ")," << std::endl;
        }
    }

    colOut << "    COL_TRI_STOP()," << std::endl;

    /* Add waterboxes */

    if (waterBox.size() > 0) {
        colOut << "    COL_WATERBOX_INIT(" << waterBox.size() << ")," << std::endl;
        for (u8 i = 0; i < waterBox.size(); i++) {
            colOut << "    COL_WATER_BOX(" << std::right << std::setw(2)
                   << (u16)waterBox[i].type << ", " << std::right << std::setw(6)
                   << waterBox[i].x1 << ", " << std::right << std::setw(6)
                   << waterBox[i].z1 << ", " << std::right << std::setw(6)
                   << waterBox[i].x2 << ", " << std::right << std::setw(6)
                   << waterBox[i].z2 << ", " << std::right << std::setw(6)
                   << waterBox[i].y << ")," << std::endl;
        }
    }

    colOut << "    COL_END()" << std::endl << "};" << std::endl;
}

/**
 * Main collision converter function.
 */

void collision_converter_main(const std::string &file, const std::string &fileOut, s16 scale) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file, aiProcess_ValidateDataStructure | aiProcess_Triangulate | aiProcess_PreTransformVertices);

    std::fstream collisionOut;
    collisionOut.open(fileOut + "/collision.inc.c", std::iostream::out | std::iostream::app);
    reset_file(fileOut + "/collision.inc.c");
    collisionOut << std::endl
                 << "const Collision " << get_filename(fileOut) << "_collision[] = {"
                 << std::endl << "    COL_INIT()," << std::endl;
    collisionOut.close();

    CollisionMat mat[scene->mNumMaterials];
    configure_materials(scene, mat);

    /* Count vtx amount, setup vtx and cleanup output */

    inspect_vtx(scene->mRootNode, scene, mat);
    CollisionVtx vtx[vertex];
    setup_vtx(file, scene->mRootNode, scene, vtx, scale);
    clean_vtx(vtx);

    /* Configure waterboxes */

    for (u8 i = 0; i < waterBox.size(); i++) {
        configure_waterbox(scene, scene->mRootNode, scale, waterBox[i]);
    }

    std::sort(waterBox.begin(), waterBox.end(), waterbox_sort);

    /* Write data */
    write_vtx(fileOut, vtx);
    write_tri(fileOut, vtx, mat);
}
