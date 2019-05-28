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

#define C_RED 0
#define C_GRN 1
#define C_BLU 2
#define C_APH 3

#include "modconv.hxx"
#include "f3d.hxx"
#include "buffer.hxx"

u32 vert     = 0,
    vert2    = 0;
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
    for (u16 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        /* we go by faces instead of verts so we don't accidentally add what we don't need */
        for (u32 j = 0; j < mesh->mNumFaces; j++) {
            printf("[dbg] face %d\n", j);
            for (u8 k = 0; k < 3; k++) {
                u32 currVtx = mesh->mFaces[j].mIndices[k];

                if (vBuf[vBuffer].isBufferComplete()) {
                    vBuf[vBuffer].vtxCount = 0;
                    vBuffer++;

                    if (vBuffer == vBuffers) { /* set the max amount for the final vbuffer */
                        vBuf[vBuffer].bufferSize = vert - vert2;
                    }
                }
                vert2++;
                u8 rgba[4] = {0xff, 0xff, 0xff, 0xff};

                if (mesh->HasVertexColors(0)) { /* Get around potential exception. */
                    rgba[C_RED] = mesh->mColors[0][currVtx].r * 0xff;
                    rgba[C_GRN] = mesh->mColors[0][currVtx].g * 0xff;
                    rgba[C_BLU] = mesh->mColors[0][currVtx].b * 0xff;
                    rgba[C_APH] = mesh->mColors[0][currVtx].a * 0xff;
                }

                s16 uv[2] = {0x00};

                /* We have to look at material data so we can multiply the UV data. */
                if (scene->HasMaterials()) { /* Ditto */
                    aiString aiPath;
                    scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &aiPath);
                    std::string path = aiPath.data;

                    if (file_exists(path)) { /* absolute */
                        std::cout << "[dbg] file exists! - " << path << std::endl;
                        uv[AXIS_X] = mesh->mTextureCoords[0][currVtx].x * 32 * get_dimension(AXIS_X, path);
                        uv[AXIS_Y] = mesh->mTextureCoords[0][currVtx].y * 32 * get_dimension(AXIS_Y, path);
                    }

                    else if (file_exists(get_path(file) + path) && !(is_directory(get_path(file) + path))) { /* relative */
                        std::cout << "[dbg] file exists (relative)! - " << file + path << std::endl;
                        uv[AXIS_X] = mesh->mTextureCoords[0][currVtx].x * 32 * get_dimension(AXIS_X, path);
                        uv[AXIS_Y] = mesh->mTextureCoords[0][currVtx].y * 32 * get_dimension(AXIS_Y, path);
                    }

                    else { /* no texture found */
                        std::cout << "[dbg] no texture found - abs " << path << " - relative - " << file + path << std::endl;
                    }
                }
                printf("[dbg] vtx %d %d %d\n", (s16)mesh->mVertices[currVtx].x * scale, (s16)mesh->mVertices[currVtx].y * scale, (s16)mesh->mVertices[currVtx].z * scale);
                vBuf[vBuffer].addVtx((s16)mesh->mVertices[currVtx].x * scale,
                        (s16)mesh->mVertices[currVtx].y * scale,
                        (s16)mesh->mVertices[currVtx].z * scale,
                        uv[AXIS_X], uv[AXIS_Y],
                        rgba[C_RED], rgba[C_GRN], rgba[C_BLU], rgba[C_APH], i);
            }
        }
    }
}

/** Write display list commands to file. */
/*
 * PROCESS:
 * 1.) Setup (Add glabel and disable G_LIGHTING)
 * 2.) Check if materials are different by looking at the mesh ids a vertex corresponds to.
 * 3.) Check if TRI2 is possible within either the vertex buffer range AND if it doesnt go into
 * a vertex with a different material.
 * 4.) End displaylist after all of that crap is done.
 */
/*static void write_display_list() {
}*/

void f3d_main(const std::string &file, const std::string &fileOut, s16 scale, u8 microcode, bool level)
{
    Assimp::Importer importer;

    /* We don't use ASSIMP's built in tristripping because of the vertex buffer. */
    const aiScene* scene = importer.ReadFile(file, aiProcess_ValidateDataStructure);
    std::string path = fileOut + "/model.s";

    count_vtx(scene->mRootNode, scene);

    printf("[dbg] there are %d verts\n", vert);
    vBuffers = vert / microcode;

    if (vert % microcode > 0) { /* is there a trailing vbuffer? */
        vBuffers++;
    }

    VertexBuffer vBuf[vBuffers];

    for (u16 i = 0; i < scene->mRootNode->mNumChildren; i++) {
        setup_vtx(scene->mRootNode->mChildren[i], scene, scale, vBuf, file);
    }
    std::cout << "[dbg] There are " << vBuffers << " vbuffers." << std::endl;
    std::cout << "[dbg] now beginning vtx test" << std::endl;

    for (u16 k = 0; k < vBuffers; k++) {
        vBuf[k].vtxCount = 0;
    }

    /* Test vertex buffer. */
    for (u16 i = 0; i < vBuffers; i++) {
        for (u8 j = 0; j < microcode - 1; j++) {
            if (!vBuf[i].isBufferComplete()) {
                std::cout << vBuf[i].vtx[j].pos[AXIS_X] << " ";
                std::cout << vBuf[i].vtx[j].pos[AXIS_Y] << " ";
                std::cout << vBuf[i].vtx[j].pos[AXIS_Z] << std::endl;
                vBuf[i].vtxCount++;
            }
        }
    }
}
