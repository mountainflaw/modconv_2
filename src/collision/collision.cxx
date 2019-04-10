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
*       * Neither the name of the Obsidian developers nor the                           *
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
#include "../common.hxx"
#include "surfaces.hxx"

s32 vertex = 0, tri = 0, vtx = 0; /* Globals */

void write_vertex(aiNode* node, const aiScene* scene, const std::string &fileOut, s16 scale)
{
    std::fstream collisionOut;
    collisionOut.open(fileOut + "/collision.s", std::iostream::out | std::iostream::app);

    for (u16 i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        for (u16 i = 0; i < mesh->mNumVertices; i++)
            collisionOut << "colVertex " << std::to_string((s16)(mesh->mVertices[i].x * scale)) << ", " << std::to_string((s16)(mesh->mVertices[i].y * scale)) << ", " << std::to_string((s16)(mesh->mVertices[i].z * scale)) << std::endl;
    }
}

void write_triangle(aiNode* node, const aiScene* scene, const std::string &fileOut)
{
    std::fstream collisionOut;
    collisionOut.open(fileOut + "/collision.s", std::iostream::out | std::iostream::app);

    for (u16 i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        std::string terrainType = "SURF_ENV_DEFAULT";

//        if (scene->hasMaterials())
//        {
            /* Search for collision name shit here */
//        }

        /* Triangle */  
        collisionOut << std::endl << "colTrisInit " << terrainType << " " << std::to_string(mesh->mNumFaces) << std::endl;

        for (u16 i = 0; i < mesh->mNumFaces; i++)
            collisionOut << "colTri " << vertex + mesh->mFaces[i].mIndices[0] << ", " << vertex + mesh->mFaces[i].mIndices[1] << ", " << vertex + mesh->mFaces[i].mIndices[2] << std::endl;

        vertex += mesh->mNumVertices;
    }
}

void set_tri_amount(aiNode* node, const aiScene* scene)
{
    for (u16 i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        tri += mesh->mNumFaces;
    }
}

void set_vtx_amount(aiNode* node, const aiScene* scene)
{
    for (u16 i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        vtx += mesh->mNumVertices;
    }
}

void collision_converter_main(const std::string &file, const std::string &fileOut, s16 scale)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file, aiProcess_ValidateDataStructure | aiProcess_JoinIdenticalVertices);

    std::fstream collisionOut;
    collisionOut.open(fileOut + "/collision.s", std::iostream::out | std::iostream::app);
    collisionOut << "glabel " << fileOut << "_collision" << std::endl << "colInit" << std::endl;

    for (u16 i = 0; i < scene->mRootNode->mNumChildren; i++)
        set_tri_amount(scene->mRootNode->mChildren[i], scene);

    for (u16 i = 0; i < scene->mRootNode->mNumChildren; i++)
        set_vtx_amount(scene->mRootNode->mChildren[i], scene);

    collisionOut << std::endl << "colVertexInit " << vtx << std::endl;
    for (u16 i = 0; i < scene->mRootNode->mNumChildren; i++)
            write_vertex(scene->mRootNode->mChildren[i], scene, fileOut, scale);

    for (u16 i = 0; i < scene->mRootNode->mNumChildren; i++)
            write_triangle(scene->mRootNode->mChildren[i], scene, fileOut);

    collisionOut << std::endl << "colTriStop" << std::endl;
    collisionOut << "colEnd" << std::endl;
}
