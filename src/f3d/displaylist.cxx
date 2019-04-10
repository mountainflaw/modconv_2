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
#include "vertex.hxx"

class Vertex;
class Material;

void build_displaylist(const std::string fileOut, Vertex *vtx, Material *mat, s32 verts, s8 f3d)
{
    s16 bVert = -1, gVert = 0, matId = -1;
    std::ofstream displayListOut;
    displayListOut.open(fileOut + "/model.s", std::ofstream::out | std::ofstream::app);

    displayListOut << std::endl << "glabel " << fileOut << "_dl" << std::endl;
    for (int i = 0; i < verts;)
    {
        /* Materials */

        if (!(vtx[i].compareMaterials(matId)))
        {
            matId++;
            displayListOut << mat[matId].getMaterial();
        }

        if (!(gVert % f3d))
        {
            gVert = 0;
            bVert++;

            if (verts - i + 1 < f3d)
                displayListOut << "gsSPVertex " << fileOut << "_vertex_" << bVert << " " << (verts - i) << ", 0" << std::endl;

            else
                displayListOut << "gsSPVertex " << fileOut << "_vertex_" << bVert << std::to_string(f3d) << ", 0" << std::endl;
        }

        displayListOut << "gsSP1Triangle " << gVert << ", " << gVert + 1 << ", " << gVert + 2 << std::endl;
        gVert += 3;
        i += 3;
    }
    displayListOut << "gsSPEndDisplayList" << std::endl;
}
