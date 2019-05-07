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

#include "../common.hxx"
#include "displaylist.hxx"

u32 vert   = 0, //! Used by vertex builder
    verts  = 0; //! Total amount of vertices
u16 meshId = 0; //! Recorded into each vert

void get_num_of_verts(aiNode* node, const aiScene* scene)
{
    for (u16 i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]]; 
        for(u32 i = 0; i < mesh->mNumVertices; i++)
            verts++;
        meshId++;
    }

    for (u16 i = 0; i < node->mNumChildren; i++)
        get_num_of_verts(node->mChildren[i], scene);
}

/** Grabs the vertex data from ASSIMP and places it inside the vertex objects. */
void setup_vtx(aiNode* node, const aiScene* scene, s16 scale, Vertex *vtx, Material *mat, const std::string &file)
{
    aiString path, name; /** Raw path and name strings */
    bool textured = false; /** Is the material textured? */

    for (u16 j = 0; j < node->mNumMeshes; j++)
    {
        if (scene->HasMaterials())
        {
            scene->mMaterials[j]->GetTexture(aiTextureType_DIFFUSE, 0, &path);
            scene->mMaterials[j]->Get(AI_MATKEY_NAME, name);

            std::string nameString = name.data,
                        pathString = path.data;

            if (std::filesystem::exists(pathString))
            {
                u16 textureWrap[2] = {0};
                textured = true;
                std::cout << "DBG - PATH EXISTS!" << std::endl;
                std::cout << pathString << std::endl;
                mat[meshId].setMaterial(get_path(file) + pathString, nameString, 0, 0, 0, textureWrap[AXIS_X], textureWrap[AXIS_Y]);
            }

            else if (file_exists(get_path(file) + pathString) && !(is_directory(get_path(file) + pathString)))
            {
                u16 textureWrap[2] = {0};
                textured = true;
                std::cout << "DBG - PATH EXISTS (RELATIVE)"
                          << get_path(file) + pathString << std::endl;

                //if (AI_SUCCESS != scene->mMaterials[j]->Get(AI_MATKEY_MAPPINGMODE_U_DIFFUSE(5), textureWrap[AXIS_X]) ||
                //                  scene->mMaterials[j]->Get(AI_MATKEY_MAPPINGMODE_V(j, aiTextureType_DIFFUSE), textureWrap[AXIS_Y]))
                //     mat[meshId].setMaterial(get_path(file) + pathString, nameString, 0, 0, 0, 0, 0);
                //else
                //{
                //    std::cout << "DBG - WARNING: FALLING BACK ON TEXWRAP MODE!" << std::endl;
                    mat[meshId].setMaterial(get_path(file) + pathString, nameString, 0, 0, 0, textureWrap[AXIS_X], textureWrap[AXIS_Y]);
                //}
                //std::cout << "DBG - TEXWRAP XY: " << std::to_string(textureWrap[AXIS_X]) << " " << std::to_string(textureWrap[AXIS_Y]) << std::endl;
            }

            else
            {
                aiColor3D pColor;
                std::cout << "DBG - TEXTURE DOES NOT EXIST! USING BASE COLORS!" <<
                "\nRelative: " << get_path(file) + pathString << "\nAbsolute: " << pathString << std::endl;

                if(AI_SUCCESS != scene->mMaterials[j]->Get(AI_MATKEY_COLOR_DIFFUSE, pColor))
                {
                    std::cout << "DBG - oh god oh fuck time to whip out the default material" << std::endl;
                    mat[meshId].setMaterial("CONV_UNUSED", nameString, 238, 238, 238, 0, 0); /* Set it to a nice #EEEEEEE if it gets fucked somehow */
                }

                else
                {
                    std::cout << "DBG - Safely using primcolors" << std::endl;
                    mat[meshId].setMaterial("CONV_UNUSED", nameString, pColor.r, pColor.g, pColor.b, 0, 0);
                }
            }
        }

        aiMesh* mesh = scene->mMeshes[node->mMeshes[j]];

        for(u32 i = 0; i < mesh->mNumVertices; i++)
        {
            vtx[vert].setPos(AXIS_X, mesh->mVertices[i].x * scale);
            vtx[vert].setPos(AXIS_Y, mesh->mVertices[i].y * scale);
            vtx[vert].setPos(AXIS_Z, mesh->mVertices[i].z * scale);

            if (mesh->HasTextureCoords(0) && textured)
            {
                vtx[vert].setTextureCoords(AXIS_X, mesh->mTextureCoords[0][i].x * 32 * mat[meshId].getDimension(AXIS_X));
                vtx[vert].setTextureCoords(AXIS_Y, mesh->mTextureCoords[0][i].y * 32 * mat[meshId].getDimension(AXIS_Y));
            }

            else
            {
                vtx[vert].setTextureCoords(AXIS_X, 0);
                vtx[vert].setTextureCoords(AXIS_Y, 0);
            }

            if (mesh->HasVertexColors(0))
            {
                vtx[vert].setColor(C_RED,   (u8)(mesh->mColors[0][i].r * 0xff));
                vtx[vert].setColor(C_GREEN, (u8)(mesh->mColors[0][i].g * 0xff));
                vtx[vert].setColor(C_BLUE,  (u8)(mesh->mColors[0][i].b * 0xff));
                vtx[vert].setColor(C_ALPHA, (u8)(mesh->mColors[0][i].a * 0xff));
            }

            else
            {
                vtx[vert].setColor(C_RED,   0xff);
                vtx[vert].setColor(C_GREEN, 0xff);
                vtx[vert].setColor(C_BLUE,  0xff);
                vtx[vert].setColor(C_ALPHA, 0xff);
            }

            vtx[vert].setMaterial(meshId);
            vert++;
        }
        meshId++;
    }
}

/** Writes vertices to file */
void write_vtx(Vertex *vtx, const std::string &fileOut, s8 output, const std::string &path)
{
    u16 vtxGroup = 0;
    std::ofstream vertexOut;
    vertexOut.open(path, std::iostream::out | std::iostream::app);

    for (u32 i = 0; i < verts; i++)
    {
        if (!(i % output)) /* F3D type */
            vertexOut << "\n" << fileOut << "_vertex_" << vtxGroup++ << ":" << std::endl;

        if (1) /* Optimizer */
        {
            vertexOut << "vertex " << vtx[i].getPos(AXIS_X) << ", " << vtx[i].getPos(AXIS_Y) << ", " << vtx[i].getPos(AXIS_Z) << ",    \t";
            vertexOut << vtx[i].getTextureCoords(AXIS_X) << ", " << vtx[i].getTextureCoords(AXIS_Y) << ",    \t" << vtx[i].getColor(C_RED) << ", "; 
            vertexOut << vtx[i].getColor(C_GREEN) << ", " << vtx[i].getColor(C_BLUE) << ", " << vtx[i].getColor(C_ALPHA) << "  \t\t# VERTEX #" << i + 1 << std::endl;
        }
    }
}

/** Writes materials to file. */
void write_materials(Material *mat, const std::string &fileOut)
{
    /* PHASE 1: Setup string array */ /* TODO: make this entire function not retarded. */
    std::string matOutputs[meshId] = {"CONV_UNUSED"};
    for (u16 i = 0; i < meshId; i++)
    {
        std::string matName = mat[i].getFileNameNoExtension();
        if (!(matName.find("CONV_UN") != std::string::npos) && mat[i].isTextured())
            matOutputs[i] = matName + ":\n" + ".incbin " + R"(")" + sanitize_output(matName) + R"(")" + "\n";
    }

    /* PHASE 2: Optimize the material string array so we don't incbin twice */
    for (u16 i = 0; i < meshId; i++)
    {
        for (u16 j = 0; j < meshId; j++)
        {
            if (matOutputs[i].compare(matOutputs[j]) == 0 && i > j)
                matOutputs[i] = "CONV_UN";
        }
    }

    /* PHASE 3: Actually write the materials */
    std::fstream materialOut;
    materialOut.open(fileOut + "/model.s", std::iostream::out | std::iostream::app);
    for (u16 i = 0; i < meshId; i++)
    {
        if (!(matOutputs[i].find("CONV_UN") != std::string::npos) && (mat[i].isTextured()))
        {
            std::cout << "DBG - a" << std::endl;
            materialOut << matOutputs[i];
        }


        if (!(matOutputs[i].find("CONV_UN") != std::string::npos) && (mat[i].isTextured())
            && (matOutputs[i].find("ci4") != std::string::npos || matOutputs[i].find("ci8") != std::string::npos)
            && mat[i].isTextured())
        {
            std::cout << "DBG - CI TEXTURE INCLUDE" << std::endl;
            std::string matName = sanitize_output(mat[i].getFileNameNoExtension());
            materialOut << matName << "_pal:" << std::endl;
            materialOut << ".incbin " << R"(")" << matName << R"(.pal")" << std::endl;
        }
    }
}

/** Main function for the vertex phase. */
void vtx_phase(const std::string &file, const std::string &fileOut, s16 scale, u8 f3d, u8 area)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file, aiProcess_ValidateDataStructure);
    std::string path;

    /* Some file operations */
    //reset_directory(fileOut);

    if (area > 0)
        path = fileOut + "/areas/" + std::to_string(area) + "/model.s"; /* Area */
    else
        path = fileOut + "/model.s"; /* Actor */

    /* Get amount of verts and create vertex objects */
    get_num_of_verts(scene->mRootNode, scene);
    Vertex vtx[verts];
    Material mat[meshId];
    meshId = 0;
    std::cout << "DBG - Number of verts: " << std::to_string(verts) << std::endl;

    for (u16 i = 0; i < scene->mRootNode->mNumChildren; i++)
        setup_vtx(scene->mRootNode->mChildren[i], scene, scale, vtx, mat, file);

    write_materials(mat, fileOut);

    /* One day: optimize verts */
    /* Output vertices */
    write_vtx(vtx, fileOut, f3d, path);

    /* Build display list */
    build_displaylist(fileOut, vtx, mat, verts, f3d);
}
