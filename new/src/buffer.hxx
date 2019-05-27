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

typedef struct
{
    s16  pos[3], st[2], mat;
    u16  flag[4]; /* optimizer flag */
    u8   col[4];
} Vertex;

/* Used in the F3D DL builder. */
class VertexBuffer
{
    private:
    bool cprVert(Vertex *a, Vertex *b)
    {
        return true;
    }

    public:
    Vertex vtx[77];
    u16 bufferIndex = 0;
    u8 vtxCount   = 0,
       bufferSize = 14;

    bool isBufferComplete() { return vtxCount == bufferSize; }

    void optimizeVerts()
    {

    }

    void addVtx(s16 vtxPosX, s16 vtxPosY, s16 vtxPosZ,
            s16 vtxPosU, s16 vtxPosV,
            u8 vtxCRed, u8 vtxCGreen, u8 vtxCBlue, u8 vtxCAlpha,
            u16 mesh)
    {
        vtx[vtxCount].pos[AXIS_X] = vtxPosX;
        vtx[vtxCount].pos[AXIS_Y] = vtxPosY;
        vtx[vtxCount].pos[AXIS_Z] = vtxPosZ;

        vtx[vtxCount].st[AXIS_X] = vtxPosU;
        vtx[vtxCount].st[AXIS_Y] = vtxPosV;

        vtx[vtxCount].col[C_RED]  = vtxCRed;
        vtx[vtxCount].col[C_GRN]  = vtxCGreen;
        vtx[vtxCount].col[C_BLU]  = vtxCBlue;
        vtx[vtxCount].col[C_APH]  = vtxCAlpha;

        vtx[vtxCount].flag[3]     = mesh;

        vtxCount++;
    }

    Vertex getVtx(u8 index)
    {
        vtxCount++;
        return vtx[index];
    }
};
