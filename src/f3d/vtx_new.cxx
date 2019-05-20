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

#include "../common.hxx"
#include "displaylist.hxx"

u32 vert     = 0;
u16 meshId   = 0,
    vBuffers = 0; //! Amount of vtx buffers

/*
 * New vertex builder
 *
 * Compared to the older vertex builder, this one
 * has many advantages:
 * 1.) It is a lot safer:
 *    - The vertices actually used by the triangles are counted,
 *      instead of overall vertices.
 *    - The vertex buffers are precalculated.
 * 2.) Because the vertex buffers are precalculated, semi-tristripping
 *     is easily achievable.
 */

/* More reliable way to count vertices. */
static void count_vtx(aiNode* node, const aiScene* scene)
{
    for (u16 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]]; 
        vert += mesh->mNumFaces * 3;
        meshId++;
    }

    for (u16 i = 0; i < node->mNumChildren; i++) {
        count_vtx(node->mChildren[i], scene);
    }
}

u16 vBufferCt = 0,
    vBufferVt = 0;
/* Materials are no longer setup here. */
static void setup_vtx(aiNode* node, const aiScene* scene, s16 scale,
    VertexBuffer* vBuffer, s16 vBuffers, const std::string &file)
{
    for (u16 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        for(u32 j = 0; j < mesh->mNumVertices; j++) {
            vBuffer[vBufferCt].vtx[vBufferVt].setPos(AXIS_X, mesh->mVertices[j].x * scale);
            vBuffer[vBufferCt].vtx[vBufferVt].setPos(AXIS_Y, mesh->mVertices[j].y * scale);
            vBuffer[vBufferCt].vtx[vBufferVt].setPos(AXIS_Z, mesh->mVertices[j].z * scale);

            if (mesh->HasTextureCoords(0)) {
                u16 uvX, uvY = 0;
                /* Get material to multiply UV data by. */
                aiString path;
                scene->mMaterials[j]->GetTexture(aiTextureType_DIFFUSE, 0, &path);
                std::string nameString = path;

                if (file_exists(get_path(path)) && !(is_directory(get_path(path)))) {
                    std::cout << "DBG - PATH EXISTS!" << std::endl;
                    uvX = get_dimension(AXIS_X, path);
                    uvY = get_dimension(AXIS_Y, path);
                }

                else if (file_exists(get_path(file) + path) && !(is_directory(get_path(file) + path))) {
                    uvX = get_dimension(AXIS_X, file + path);
                    uvY = get_dimension(AXIS_Y, path + path);
                }

                vBuffer[vBufferCt].vtx[vBufferVt].setTextureCoords(AXIS_X, mesh->mTextureCoords[0][j].x * 32 * mat[meshId].getDimension(AXIS_X));
                vBuffer[vBufferCt].vtx[vBufferVt].setTextureCoords(AXIS_Y, mesh->mTextureCoords[0][j].y * 32 * mat[meshId].getDimension(AXIS_Y));
            }

            else {
                vBuffer[vBufferCt].vtx[vBufferVt].setTextureCoords(AXIS_X, 0);
                vBuffer[vBufferCt].vtx[vBufferVt].setTextureCoords(AXIS_Y, 0);
            }

            if (mesh->HasVertexColors(0)) {
                vBuffer[vBufferCt].vtx[vBufferVt].setColor(C_RED,   (u8)(mesh->mColors[0][j].r * 0xff));
                vBuffer[vBufferCt].vtx[vBufferVt].setColor(C_GREEN, (u8)(mesh->mColors[0][j].g * 0xff));
                vBuffer[vBufferCt].vtx[vBufferVt].setColor(C_BLUE,  (u8)(mesh->mColors[0][j].b * 0xff));
                vBuffer[vBufferCt].vtx[vBufferVt].setColor(C_ALPHA, (u8)(mesh->mColors[0][j].a * 0xff));
            }

            else {
                vBuffer[vBufferCt].vtx[vBufferVt].setColor(C_RED,   0xff);
                vBuffer[vBufferCt].vtx[vBufferVt].setColor(C_GREEN, 0xff);
                vBuffer[vBufferCt].vtx[vBufferVt].setColor(C_BLUE,  0xff);
                vBuffer[vBufferCt].vtx[vBufferVt].setColor(C_ALPHA, 0xff);
            }
        }
    }
}

/** Writes materials to file. */
static void write_materials(Material *mat, const std::string &fileOut, u8 area)
{
    /* PHASE 1: Setup string array */ /* TODO: make this entire function not retarded. */
    std::string matOutputs[meshId] = {"CONV_UNUSED"};
    for (u16 i = 0; i < meshId; i++) {
        std::string matName = mat[i].getFileNameNoExtension();
        if (!(matName.find("CONV_UN") != std::string::npos) && mat[i].isTextured())
            matOutputs[i] = matName + ":\n" + ".incbin " + R"(")" + sanitize_output(matName) + R"(")" + "\n";
    }

    /* PHASE 2: Optimize the material string array so we don't incbin twice */
    for (u16 i = 0; i < meshId; i++) {
        for (u16 j = 0; j < meshId; j++) {
            if (matOutputs[i].compare(matOutputs[j]) == 0 && i > j) {
                matOutputs[i] = "CONV_UN";
            }
        }
    }

    /* PHASE 3: Actually write the materials */
    std::fstream materialOut;

    if (area > 0) {
        materialOut.open(fileOut + "/texture.s", std::iostream::out | std::iostream::app);
    }

    else materialOut.open(fileOut + "/model.s", std::iostream::out | std::iostream::app);

    for (u16 i = 0; i < meshId; i++) {
        if (!(matOutputs[i].find("CONV_UN") != std::string::npos) && (mat[i].isTextured())) {
            std::cout << "DBG - a" << std::endl;
            //materialOut << matOutputs[i];

            if (matOutputs[i].find("ci4") != std::string::npos || matOutputs[i].find("ci8") != std::string::npos) {
                std::cout << "DBG - CI TEXTURE INCLUDE" << std::endl;
                std::string matName = mat[i].getFileNameNoExtension();
                materialOut << matName << "_pal:" << std::endl;
                materialOut << ".incbin " << R"(")" << matName << R"(.pal")" << std::endl;
            }
        }
    }
}

void vtx_main(const std::string &file, const std::string &fileOut, s16 scale, u8 f3d, u8 area)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file, aiProcess_ValidateDataStructure);
    count_vtx(scene->mRootNode, scene);

    u16 vBuffers = vert / f3d;

    if (vert % (f3d * (vert / f3d)) > 0) { /* is there a trailing vbuffer? */
        vBuffers++;
    }

    Material     mat[meshId];
    VertexBuffer vBuffer[vBuffers];

    for (u16 i = 0; i < vBuffers; i++) {
        vBuffer[i].bufferSize = f3d;
    }
}
