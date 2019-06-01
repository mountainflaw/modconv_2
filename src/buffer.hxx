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

enum MaterialEnum { MATERIAL, OLDPOS, NEWPOS};

typedef struct
{
    s16  pos[3], st[2];
    s16  flag[3]; /* optimizer flag */
    u8   col[4];
    bool useless;
} Vertex;

/* Used in the F3D DL builder. */
class VertexBuffer
{
    private:
    Vertex vtx[78];

    bool cprVert(Vertex *vtx, u8 i, u8 k)
    {
        bool cprPos = ((vtx[i].pos[AXIS_X] == vtx[k].pos[AXIS_X]) && (vtx[i].pos[AXIS_Y] == vtx[k].pos[AXIS_Y])
                && (vtx[i].pos[AXIS_Z] == vtx[k].pos[AXIS_Z]));

        bool cprSt = ((vtx[i].st[AXIS_X] == vtx[k].st[AXIS_X]) && (vtx[i].st[AXIS_Y] == vtx[k].st[AXIS_Y]));

        bool cprCol = ((vtx[i].col[AXIS_X] == vtx[k].col[AXIS_X]) && (vtx[i].col[AXIS_Y] == vtx[k].col[AXIS_Y])
            && (vtx[i].col[AXIS_Y] == vtx[k].col[AXIS_Y]) && (vtx[i].col[AXIS_Y] == vtx[k].col[AXIS_Y]));
        return cprPos && cprSt && cprCol;
    }

    u16 getVtxMatTri2() { return vtx[vtxCount + 3].flag[MATERIAL]; }

    public:
    u8 vtxCount   = 0,
       bufferSize = 15;

    bool isBufferComplete() { return vtxCount == bufferSize; }

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

        vtx[vtxCount].useless = false;
        vtx[vtxCount].flag[MATERIAL]     = mesh;

        vtxCount++;
    }

    void optimizeVerts()
    {
        /* Stage 1: Mark redundant vertices. */
        for (u8 i = 0; i < bufferSize; i++) {
            for (u8 k = 0; k < bufferSize; k++) {
                if (cprVert(vtx, i, k) && k > i) {
                    vtx[k].useless = true;
                    vtx[k].flag[OLDPOS] = i;
                }
            }
        }

        /* Stage 2: Calculate the left behind verts' new position. */
        for (u8 i = 0; i < bufferSize; i++) {
            if (!vtx[i].useless) {
                vtx[i].flag[NEWPOS] = i;
            }
        }
    }

    Vertex getVtx() { return vtx[vtxCount++]; }

    u16 getVtxIndex() { return vtxCount++; }

    u16 getVtxMat() { return vtx[vtxCount].flag[MATERIAL]; }

    bool canTri2()
    { return (vtxCount + 6 <= bufferSize) && (vtx[vtxCount + 3].flag[MATERIAL] == vtx[vtxCount].flag[MATERIAL]); }
};
