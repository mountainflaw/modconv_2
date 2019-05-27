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

#include <modconv.h>
#include "f3d.hxx"

u32 vert     = 0;
u16 meshId   = 0,
    vBuffers = 0,
    vBuffer  = 0;

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

/** Add vertices to vertex buffers. */
static void setup_vtx(aiNode *node, const aiScene* scene, s16 scale,
        VertexBuffer* vBuf, const std::string &file)
{
    for (u16 i = 0; i < node-mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node-mMeshes[i]];
        bool textured = true;

        /* we go by faces instead of verts so we don't accidentally add what we don't need */
        for (u32 j = 0; j < mesh->mNumFaces; j++) {
            for (u8 k = 0; k < 2; k++) { /* We are able to assume every face is a triangle, 3 verts. */
                if (vBuf.isBufferComplete()) {
                    vBuf[vBuffer].vtxCount = 0;
                    vBuffer++;
                }

                u8[4] rgba = {0xff, 0xff, 0xff, 0xff};

                if (mesh->HasVertexColors(0)) { /* Get around potential exception. */
                    rgba[C_RED] = mesh->mColors[0][mesh->mFaces[j].mIndices[k]].r * 0xff;
                    rgba[C_GRN] = mesh->mColors[0][mesh->mFaces[j].mIndices[k]].g * 0xff;
                    rgba[C_BLU] = mesh->mColors[0][mesh->mFaces[j].mIndices[k]].b * 0xff;
                    rgba[C_APH] = mesh->mColors[0][mesh->mFaces[j].mIndices[k]].a * 0xff;
                }

                s16 uv[2] = {0x00};

                /* We have to look at material data so we can multiply the UV data. */
                if (scene->HasMaterials()) { /* Ditto */
                    aiString aiPath;
                    scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &aiPath);
                    std::string path = path.data;

                    if (file_exists(path)) { /* absolute or relative paths */
                        std::cout << "[dbg] file exists! - " << path << std::endl;
                        mesh->mTextureCoords[0][mesh->mFaces[j].mIndices[k]].x * 32 * get_dimension(AXIS_X, path);
                        mesh->mTextureCoords[0][mesh->mFaces[j].mIndices[k]].y * 32 * get_dimension(AXIS_Y, path);
                    }

                    else if (file_exists(get_path(file) + path) && !(is_directory(get_path(file) + path))) {
                        std::cout << "[dbg] file exists (relative)! - " << file + path << std::endl;
                        mesh->mTextureCoords[0][mesh->mFaces[j].mIndices[k]].x * 32 * get_dimension(AXIS_X, path);
                        mesh->mTextureCoords[0][mesh->mFaces[j].mIndices[k]].y * 32 * get_dimension(AXIS_Y, path);
                    }

                    else {
                        std::cout << "[dbg] using primcolor instead - abs " << path << " - relative - " << file + path << std::endl;
                        textured = false;
                    }
                }

                vBuf[vBuffer].addVtx(mesh->mVertices[mesh->mFaces[j].mIndices[k]].x,
                        mesh->mVertices[mesh->mFaces[j].mIndices[k]].y,
                        mesh->mVertices[mesh->mFaces[j].mIndices[k]].z,
                        uv[AXIS_X], uv[AXIS_Y],
                        rgba[C_RED], rgba[C_GRN], rgba[C_BLU], rgba[C_APH], i);
            }
        }
    }
}

static void write_display_list() {
    printf("dead\n");
}

void vtx_main(const std::string &file, const std::string &fileOut, s16 scale, u8 microcode, bool level)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file, aiProcess_ValidateDataStructure);
    std::string path = fileOut + "/model.s";

    vBuffers = vert / microcode;

    if (vert % (microcode * (vert / microcode)) > 0) { /* is there a trailing vbuffer? */
        vBuffers++;
    }

    std::cout << "[dbg] There are " << vBuffers << " vbuffers." << std::endl;
    std::cout << "[dbg] now beginning vtx test" << std::endl;
}
