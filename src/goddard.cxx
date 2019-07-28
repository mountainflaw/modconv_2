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
 * Goddard (Mario head) mesh converter
 * Author: Red
 */

#include "modconv.hxx"

static u32 vertex = 0,
      internalVtx = 0;

typedef struct {
    s16 pos[3];
    u32 list;
    u16 material;
    bool useless;
} GoddardVtx;

typedef struct {
    u16 tri;
    u16 vtx;
    f32 diffuse[3];
    u16 index;
    std::string name;
} GoddardMat;

static void inspect_vtx(const aiScene* scene, const aiNode* node, GoddardMat* mat) {
    for (u16 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        vertex += mesh->mNumFaces * 3;
        mat[mesh->mMaterialIndex].tri += mesh->mNumFaces;
    }

    for (u16 i = 0; i < node->mNumChildren; i++) {
        inspect_vtx(scene, node, mat);
    }
}

static void configure_materials(const aiScene* scene, GoddardMat* mat) {
    aiString aiName;
    aiColor4D aiDiffuse;
    for (u16 i = 0; i < scene->mNumMaterials; i++) {
        mat[i].index = i;

        if (scene->mMaterials[i]->Get(AI_MATKEY_COLOR_DIFFUSE, aiDiffuse) == AI_SUCCESS) {
            mat[i].diffuse[C_RED] = aiDiffuse.r;
            mat[i].diffuse[C_GRN] = aiDiffuse.g;
            mat[i].diffuse[C_BLU] = aiDiffuse.b;
        } else { /* no diffuse colors */
            mat[i].diffuse[C_RED] = 1.0f;
            mat[i].diffuse[C_GRN] = 1.0f;
            mat[i].diffuse[C_BLU] = 1.0f;
        }

        scene->mMaterials[i]->Get(AI_MATKEY_NAME, aiName);
        mat[i].name = aiName.data;
    }
}

static void setup_vtx(aiNode* node, const aiScene* scene, GoddardVtx* vtx, const u16 material, const s16 scale) {
    for (u16 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        if (mesh->mMaterialIndex) {
            /* We go by faces, so we don't add loose geometry to our output. */
            for (u32 j = 0; j < mesh->mNumFaces; j++) {
                for (u8 k = 0; k < 3; k++) {
                    u32 currVtx = mesh->mFaces[j].mIndices[k];
                    vtx[material][internalVtx].pos[AXIS_X] = (s16)(((mesh->mVertices[currVtx].x) * scale) * scaling_hack());
                    vtx[material][internalVtx].pos[AXIS_Y] = (s16)(((mesh->mVertices[currVtx].y) * scale) * scaling_hack());
                    vtx[material][internalVtx].pos[AXIS_Z] = (s16)(((mesh->mVertices[currVtx].z) * scale) * scaling_hack());

                    vtx[material][internalVtx].useless = false;
                    vtx[material][internalVtx].list = internalVtx;
                    vtx[material][internalVtx].material = mesh->mMaterialIndex;
                    internalVtx++;
                }
            }
        }
    }

    for (u16 i = 0; i < node->mNumChildren; i++) {
        setup_vtx(node->mChildren[i], scene, vtx, material, scale);
    }
}

static INLINE bool cprVtx(const GoddardVtx* vtx, const u16 m, const u32 i, const u32 j) {
    return abs(vtx[m][i].pos[AXIS_X] - vtx[m][j].pos[AXIS_X]) <= leniencyFactor &&
           abs(vtx[m][i].pos[AXIS_Y] - vtx[m][j].pos[AXIS_Y]) <= leniencyFactor &&
           abs(vtx[m][i].pos[AXIS_Z] - vtx[m][j].pos[AXIS_Z]) <= leniencyFactor;

}

static void clean_vtx(GoddardVtx* vtx, const GoddardMat* mat, const u16 m) {
    /* Stage 1 - Mark redundant vertices */
    for (u32 i = 0; i < mat[m].vtx; i++) {
        for (u32 j = 0; j < vertex; j++) {
            if (cprVtx(vtx, m, i, j) && j > i && !vtx[m][j].useless) {
                vtx[m][j].useless = true;
                vtx[m][j].list = i;
            }
        }
    }

    /* Stage 2 - Give verts that will be written their correct index. */
    for (u32 i = 0; i < vertex; i++) {
        if (!vtx[m][i].useless) {
            vtx[m][i].list = writeSize;
            writeSize++;
        }
    }
}

static void write_dynlist(const std::string &fileOut, const aiScene* scene, const GoddardVtx* vtx, const GoddardMat* mat, const u16 material) {
    std::string outputFile = fileOut + "/" + get_filename(fileOut) + "_goddard_mesh_mat_";

    std::fstream goddardOut;

    for (u16 i = 0; i < scene->mNumMaterials; i++) {
        reset_file(outputFile + std::to_string(i));
        goddardOut.open(outputFile + std::to_string(i), std::ofstream::out | std::ofstream::app);

        goddardOut << std::endl << "/* Material: " << mat[i].name << " */" << std::endl << std::endl;

        for (u16 j = 0; mat[i].vtx; j++) {

        }
    }
}

void goddard_main(const std::string &file, const std::string &fileOut, const s16 scale) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file, aiProcess_ValidateDataStructure | aiProcess_Triangulate | aiProcess_PreTransformVertices);

    GoddardMat mat[scene->mNumMaterials];
    inspect_vtx(scene, scene->mRootNode, mat);
    configure_materials(scene, mat);
    GoddardVtx vtx[scene->mNumMaterials][vtx];

    for (u16 i = 0; i < scene->mNumMaterials; i++) {
        setup_vtx(scene->mRootNode, scene, vtx, i, scale);
        internalVtx = 0;
    }
}
