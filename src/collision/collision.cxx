/* This file handles creating collision data out of a model imported with ASSIMP. */
#include "../common.hxx"

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
