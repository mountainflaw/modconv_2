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

/*
 * New Fast3D display list builder
 *
 * This is much more sanely built and is modeled after the collision and goddard
 * converters, which utilize the face structs instead of making assumptions about
 * vertex orders.
 *
 * The goals accomplished by this are the following:
 * 1.) Do not assume vertex order at all.
 * 2.) Implement TRI2 support.
 * 3.) Implement the vertex optimizer.
 */

#include "f3d.hxx"

u8 approachVtx = 0;

static inline void write_dl(const std::string &fileOut, aiNode* node, u16 nodeId, const aiScene* scene)
{
    u8 maxVtx = output;
    std::fstream dlOut;
    collisionOut.open(fileOut + "/model.s", std::iostream::out | std::iostream::app);

    for (u16 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        dlOut << mats[i].getMaterial();

        for (u16 j = 0; j < mesh->mNumFaces; j++) {

            dlOut << "gsSP1Triangle " << get_vtx(nodeId, i, mesh->mFaces[j].mIndices[0],
                    mesh->mFaces[j].mIndices[1], mesh->mFaces[j].mIndices[2]) << std::endl;
        }
    }
}

void displaylist_builder()
{
    write_dl();
}
