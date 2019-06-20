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
#include "surfaces.hxx"

s32 vertex = 0, tri = 0, vtx = 0; /* Globals */

static void write_vertex(aiNode* node, const aiScene* scene, const std::string &fileOut, s16 scale)
{
    std::fstream collisionOut;
    collisionOut.open(fileOut + "/collision.s", std::iostream::out | std::iostream::app);

    for (u16 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        for (u16 i = 0; i < mesh->mNumVertices; i++) {
            collisionOut << "colVertex "
                << std::to_string((s16)((mesh->mVertices[i].x * scale) * 0.01)) << ", "
                << std::to_string((s16)((mesh->mVertices[i].y * scale) * 0.01)) << ", "
                << std::to_string((s16)((mesh->mVertices[i].z * scale) * 0.01)) << std::endl;
        }
    }
}

static void write_triangle(aiNode* node, const aiScene* scene, const std::string &fileOut)
{
    std::fstream collisionOut;
    collisionOut.open(fileOut + "/collision.s", std::iostream::out | std::iostream::app);

    for (u16 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        /* TODO: Add smarter parsing here. */
        std::string terrainType = "SURF_ENV_DEFAULT";
        aiString aiName;
        scene->mMaterials[i]->Get(AI_MATKEY_NAME, aiName);
        std::string nameStr = aiName.data;

        if (scene->HasMaterials()) {
            std::cout << "material: " << nameStr << std::endl;
            for (u16 j = 0; j < SURFACES; j++) {
                if (nameStr.find(surfaces[j]) != std::string::npos) {
                    terrainType = surfaces[j].substr(1, surfaces[j].length());
                }
            }
        }

        /* Triangle */
        collisionOut << std::endl << "colTriInit " << terrainType << " " << std::to_string(mesh->mNumFaces) << std::endl;

        for (u16 j = 0; j < mesh->mNumFaces; j++) {
            collisionOut << "colTri " << vertex + mesh->mFaces[j].mIndices[0] << ", " << vertex + mesh->mFaces[j].mIndices[1] << ", " << vertex + mesh->mFaces[j].mIndices[2] << std::endl;
        }

        vertex += mesh->mNumVertices;
    }

    for (u16 i = 0; i < node->mNumChildren; i++) {
        write_triangle(node->mChildren[i], scene, fileOut);
    }
}

static void set_vtx_amount(aiNode* node, const aiScene* scene)
{
    for (u16 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        vtx += mesh->mNumVertices;
    }

    for (u16 i = 0; i < node->mNumChildren; i++) {
        set_vtx_amount(node->mChildren[i], scene);
    }
}

void collision_converter_main(const std::string &file, const std::string &fileOut, s16 scale)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file, aiProcess_ValidateDataStructure | aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_PreTransformVertices);

    std::fstream collisionOut;
    collisionOut.open(fileOut + "/collision.s", std::iostream::out | std::iostream::app);
    reset_file(fileOut + "/collision.s");
    collisionOut << "glabel " << get_filename(fileOut) << "_collision" << std::endl << "colInit";

    set_vtx_amount(scene->mRootNode, scene);

    collisionOut << std::endl << "colVertexInit " << vtx << std::endl;
    write_vertex(scene->mRootNode, scene, fileOut, scale);
    write_triangle(scene->mRootNode, scene, fileOut);

    collisionOut << std::endl << "colTriStop" << std::endl;
    collisionOut << "colEnd" << std::endl;
}
