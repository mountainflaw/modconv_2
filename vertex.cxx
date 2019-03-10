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

/* This file handles the vertex data from ASSIMP, processes it, and then outputs it to a file */

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>

#include "include.hxx"

int vert = 0;
int verts = 0;

/* These are used in displaylist.cxx */

bool g_lighting = false;
bool texcoords  = false;

int getNumVertices(aiNode* node, const aiScene* scene)
{
    for (int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]]; 

        for(int i = 0; i < mesh->mNumVertices; i++)
            verts++;
    }

    for (int i = 0; i < node->mNumChildren; i++)
        getNumVertices(node->mChildren[i], scene);

    return verts;
}

/* Required because ASSIMP sucks and requires you to go through each node.
 * On the upside this probably fixes split meshes. */

void processNode(aiNode* node, const aiScene* scene, int scale, struct vertex *vtx, std::string output)
{
    for (int j = 0; j < node->mNumMeshes; j++)
    {
        aiString path;
        scene->mMaterials[j]->GetTexture(aiTextureType_DIFFUSE, 0, &path);
        std::cout << path.data << std::endl;
        std::filesystem::copy(path.data, output, std::filesystem::copy_options::overwrite_existing);
        aiMesh* mesh = scene->mMeshes[node->mMeshes[j]];

        for(int i = 0; i < mesh->mNumVertices; i++)
        {
            vtx[vert].vertPos[AXIS_X] = (int)(mesh->mVertices[i].x * scale);
            vtx[vert].vertPos[AXIS_Y] = (int)(mesh->mVertices[i].y * scale);
            vtx[vert].vertPos[AXIS_Z] = (int)(mesh->mVertices[i].z * scale);
            
            /* Because ASSIMP will actually throw an exception if we don't check these for whatever reason */
            
            if (mesh->HasTextureCoords(0))
            {
                vtx[vert].uv[AXIS_U] = (int)(mesh->mTextureCoords[0][i].x * 32 * 32); /* Temporary: Will be replaced by actual axis length soon */
                vtx[vert].uv[AXIS_V] = (int)(mesh->mTextureCoords[0][i].y * 32 * 32);
            }

            else
            {
                vtx[vert].uv[AXIS_U] = 0x00;
                vtx[vert].uv[AXIS_V] = 0x00;
            }
            
            /* Ditto */

            if (mesh->HasVertexColors(0))
            {
                vtx[vert].rgba[CHANNEL_RED]   = (int)(mesh->mColors[0][i].r * 0xff);
                vtx[vert].rgba[CHANNEL_GREEN] = (int)(mesh->mColors[0][i].g * 0xff);
                vtx[vert].rgba[CHANNEL_BLUE]  = (int)(mesh->mColors[0][i].b * 0xff);
                vtx[vert].rgba[CHANNEL_ALPHA] = (int)(mesh->mColors[0][i].a * 0xff);
            }

            else
            {
                vtx[vert].rgba[CHANNEL_RED]   = 0xff;
                vtx[vert].rgba[CHANNEL_BLUE]  = 0xff;
                vtx[vert].rgba[CHANNEL_GREEN] = 0xff;
                vtx[vert].rgba[CHANNEL_ALPHA] = 0xff;
                g_lighting = true;
            }

            /* Ditto */
            if (mesh->HasNormals())
                vtx[vert].norm = (int)(mesh->mNormals[0][i]);

            else
                vtx[vert].norm = VTX_NORMAL_NONE; /* Fallback case */

            vtx[vert].mesh = j;            

            vtx[vert].map = VTX_DONT_SKIP; /* Optimizer default */
            vert++;


            /* Test output */

            #ifdef DEBUG_OUTPUT
            printf("TO %d %d %d %d\n", (int)(mesh->mVertices[i].x * scale), (int)(mesh->mVertices[i].y * scale), (int)(mesh->mVertices[i].z * scale), vert);
            #endif
        }
    }
    return;
}

/* Final stage in vertex process */

void writeVertices(struct vertex *vtx, std::string fileOut, int tVerts)
{
    int vtxGroup = 0;
    char fileOutChar[fileOut.length()];
    strcpy(fileOutChar, fileOut.c_str());
    std::ofstream vertexOut;
    std::string fileName = fileOut + "/actor.s";

    /* Thank you Microsoft, very cool! */

    #ifdef _WIN32
        _mkdir(fileOutChar);
    #else
        mkdir(fileOutChar, 0777);
    #endif

    vertexOut.open(fileName, std::iostream::out);
    vertexOut << "# Generated by obsidian, written by Red.\n";
    for (int i = 0; i < tVerts; i++)
    {
        if (!(i % 15))
            vertexOut << "\n" << fileOut << "_vertex_" << vtxGroup++ << ":" << std::endl;
        if (vtx[i].map == VTX_DONT_SKIP) /* Optimizer */
        {
            vertexOut << "vertex " << vtx[i].vertPos[AXIS_X] << ", " << vtx[i].vertPos[AXIS_Y] << ", " << vtx[i].vertPos[AXIS_Z] << ",    \t";
            vertexOut << vtx[i].uv[AXIS_U] << ", " << vtx[i].uv[AXIS_V] << ",    \t" << vtx[i].rgba[CHANNEL_RED] << ", "; 
            vertexOut << vtx[i].rgba[CHANNEL_GREEN] << ", " << vtx[i].rgba[CHANNEL_BLUE] << ", " << vtx[i].rgba[CHANNEL_ALPHA] << "  \t\t# VERTEX #" << i + 1 << std::endl;
            
            #ifdef DEBUG2
            printf("BACK: vert xyz i rgba %d %d %d\t\t  %d\t\t  %d %d %d %d\n", vtx[i].vertPos[AXIS_X], vtx[i].vertPos[AXIS_Y], vtx[i].vertPos[AXIS_Z], i + 1, vtx[i].rgba[CHANNEL_RED], vtx[i].rgba[CHANNEL_RED], vtx[i].rgba[CHANNEL_GREEN], vtx[i].rgba[CHANNEL_BLUE], vtx[i].rgba[CHANNEL_ALPHA]);
            #endif
        }
    }
    return;
}

/* Compares verts to check if they're duplicates */

bool compareVerts(struct vertex *vtx, int i, int j)
{
    #ifdef DEBUG
    puts("COMPARING VERTS");
    #endif
    bool matchPos     = false, 
         matchUvs     = false, 
         matchColors  = false, 
         matchNormals = false;

    if ((vtx[i].vertPos[AXIS_X]    == vtx[j].vertPos[AXIS_X]) 
        && (vtx[i].vertPos[AXIS_Y] == vtx[j].vertPos[AXIS_Y]) 
        && (vtx[i].vertPos[AXIS_Z] == vtx[j].vertPos[AXIS_Z]) && i != j)
    {
        matchPos = true;
        #ifdef DEBUG_OPTIMIZER
        printf("[DBG] Positions matched! (%d %d %d vs %d %d %d)\n", vtx[i].vertPos[AXIS_X], vtx[i].vertPos[AXIS_Y] , vtx[i].vertPos[AXIS_Z],
        vtx[j].vertPos[AXIS_X], vtx[j].vertPos[AXIS_Y], vtx[j].vertPos[AXIS_Z]);
        #endif
    }
    
    if ((vtx[i].uv[AXIS_U]    == vtx[j].uv[AXIS_U])
        && (vtx[i].uv[AXIS_V] == vtx[j].uv[AXIS_V]) && i != j)
    {
        matchUvs = true;
        #ifdef DEBUG_OPTIMIZER
        printf("[DBG] UVs matched! %d %d vs %d %d\n", vtx[i].uv[AXIS_U], vtx[i].uv[AXIS_V], vtx[j].uv[AXIS_U], vtx[j].uv[AXIS_V]);
        #endif
    }

    if ((vtx[i].rgba[CHANNEL_RED]      == vtx[j].rgba[CHANNEL_RED])
        && (vtx[i].rgba[CHANNEL_GREEN] == vtx[j].rgba[CHANNEL_GREEN])
        && (vtx[i].rgba[CHANNEL_BLUE]  == vtx[j].rgba[CHANNEL_BLUE])
        && (vtx[i].rgba[CHANNEL_ALPHA] == vtx[j].rgba[CHANNEL_ALPHA]) && i != j)
        {
            matchColors = true;
            #ifdef DEBUG_OPTIMIZER
            printf("[DBG] Colors matched! %d %d %d %d vs %d %d %d %d\n", vtx[i].rgba[CHANNEL_RED], vtx[i].rgba[CHANNEL_GREEN],
            vtx[i].rgba[CHANNEL_BLUE], vtx[i].rgba[CHANNEL_ALPHA], vtx[j].rgba[CHANNEL_RED], vtx[j].rgba[CHANNEL_GREEN],
            vtx[j].rgba[CHANNEL_BLUE], vtx[j].rgba[CHANNEL_ALPHA]);
            #endif
        }
    
    if(vtx[i].norm == vtx[j].norm && i != j)
    {
        matchNormals = true;
        #ifdef DEBUG_OPTIMIZER
        puts("[DBG] Normals matched!");
        #endif
    }

    if (matchPos && matchUvs && matchColors && matchNormals && vtx[i].map == -1 && j < i)
    {
        #ifdef DEBUG_OPTIMIZER
        printf("[DBG] VERTEX %d IS IDENTICAL TO %d.\n", i, j);
        #endif
        return true;
    }

    else
    {
        #ifdef DEBUG_OPTIMIZER
        puts("[DBG] VERTEX DID NOT MATCH.");
        #endif
        return false;
    }
}

/* Optimize our vertices per vertex buffer group (varies depending on graphics microcode) */

void optimizeVertices(struct vertex *vtx, int tVerts)
{
    int order = 0;
    for (int i = 0; i < 14; i++)
    {
        for (int j = 0; j < 14; j++)
        {
                if (compareVerts(vtx, i, j) && i != j)
                {
                    vtx[i].map = order;
                    order++;
                }
            }
    }
    return;
}

void prepareVertices(std::string file, std::string fileOut, int scale, int f3d)
{
    infoMessage(INFO_PREPARING_VTX);

    /* Get vertices */

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_ValidateDataStructure);
    const int tVerts = getNumVertices(scene->mRootNode, scene);
    printf("verts: %d\n", tVerts);
    /* Warn if amount of vertices can potentially cause performance issues on real hardware */

    if (tVerts >= WARNING_VERTS_NUMBER)
        warnMessage(WARNING_LARGE_VERTCOUNT, tVerts, 0, 0, 0);

    struct vertex vert[tVerts];
    
    for (int i = 0; i < scene->mRootNode->mNumChildren; i++)
        processNode(scene->mRootNode->mChildren[i], scene, scale, vert, fileOut); /* Red DESTROYS Node Meme (NOT CLICKBAIT) */
       
    /* Test output */

    #ifdef DEBUG_OUTPUT
    for (int i = 0; i < tVerts; i++)    
        printf("BACK: vert xyz i rgba %d %d %d\t\t  %d\t\t  %d %d %d %d\n", vert[i].vertPos[AXIS_X], vert[i].vertPos[AXIS_Y], vert[i].vertPos[AXIS_Z], i + 1, vert[i].rgba[CHANNEL_RED], vert[i].rgba[CHANNEL_RED], vert[i].rgba[CHANNEL_GREEN], vert[i].rgba[CHANNEL_BLUE], vert[i].rgba[CHANNEL_ALPHA]); */
    #endif

    /* Optimize vertices */

    //optimizeVertices(vert, tVerts);

    /* Submit vertices to writing function */
    infoMessage(INFO_BUILDING_VTX);
    writeVertices(vert, fileOut, tVerts);
    infoMessage(INFO_FINISHED_VTX);
    prepareDisplayList(fileOut, vert, f3d, tVerts);
    return;
}