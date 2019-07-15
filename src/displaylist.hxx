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

#include "modconv.hxx"
enum GeoModes { ENVMAP, LIN_ENVMAP, LIGHTING, SHADE, BACKFACE};

/*
 * Display list class
 * Used to build the up to 8 different types of displaylists.
 */

#define MAT_NOT_LAYER -2

class DisplayList {
    private:
    std::string dlTypes[8] = {"_layer_0", "_layer_1", "_layer_2", "_layer_3", "_layer_4", "_layer_5", "_layer_6", "_layer_7"};
    u8 layer = 1;
    bool twoCycle = false;

    bool WriteTri(s16 tri[], u8 size) {
        for (u8 i = 0; i < size; i++) {
            if (tri[i] == -1) {
                return false;
            }
        }
        return true;
    }

    public:
    void setLayer(u8 l) { layer = l; }

    void writeDisplayList(const std::string &fileOut, VertexBuffer *vBuf, u16 vBuffers, Material* mat) {
        bool oldGeo[5] = {false};
        std::fstream gfxOut;
        s16 currMat = -1; /* force update at start*/

        gfxOut.open(fileOut + "/model.s", std::ofstream::out | std::ofstream::app);
        gfxOut << std::endl << "glabel " << fileOut + dlTypes[layer] << std::endl
               << "gsSPClearGeometryMode G_LIGHTING" << std::endl;

        if (twoCycle || fog) {
            gfxOut << "gsDPSetCycleType G_CYC_2CYCLE" << std::endl;
        }

        if (fog) {
            twoCycle = true; /* so G_CYC_2CYCLE is disabled */

            if (layer > 1) { /* transparency */
                gfxOut << "gsDPSetRenderMode G_RM_FOG_SHADE_A, G_RM_AA_ZB_XLU_SURF2" << std::endl;
            } else { /* 0 and 1 */
                gfxOut << "gsDPSetRenderMode G_RM_FOG_SHADE_A, G_RM_AA_ZB_OPA_SURF2" << std::endl;
            }

            gfxOut << "gsSPSetGeometryMode G_FOG" << std::endl
                   << "gsSPFogPosition " << fogSettings[4] << ", " << fogSettings[5] << std::endl
                   << "gsDPSetFogColor " << fogSettings[0] << ", " << fogSettings[1] << ", "
                                         << fogSettings[2] << ", " << fogSettings[3] << std::endl;
        }

        for (u16 i = 0; i < vBuffers; i++) {
            vBuf[i].vtxCount = 0; /* reset this from the last layer */
            if (vBuf[i].hasLayer(layer)) { /* don't load what we don't need */
                if (vBuf[i].getLayeredVtxMat(layer) != currMat && vBuf[i].getLayeredVtxMat(layer) != MAT_NOT_LAYER) { /* load before vtx load if possible */
                    currMat = vBuf[i].getLayeredVtxMat(layer);
                    gfxOut << "/* " << mat[currMat].getName() << " */" << std::endl
                                    << mat[currMat].getMaterial(oldGeo, layer, twoCycle);
                }

                gfxOut << "gsSPVertex " << get_filename(fileOut) << "_vertex_" << i << " " << std::to_string(vBuf[i].loadSize) << ", 0" << std::endl;

                while (!vBuf[i].isBufferComplete()) {
                    if (vBuf[i].getVtxMat() != currMat && vBuf[i].getLayeredVtxMat(layer) != MAT_NOT_LAYER) {
                        currMat = vBuf[i].getLayeredVtxMat(layer);
                        bool resetVtxCache = mat[currMat].getLighting(oldGeo);
                        gfxOut << "/* " << mat[currMat].getName() << " */" << std::endl
                                        << mat[currMat].getMaterial(oldGeo, layer, twoCycle);

                        if (resetVtxCache) {
                            gfxOut << "gsSPVertex " << get_filename(fileOut) << "_vertex_" << i << " " << std::to_string(vBuf[i].loadSize) << ", 0" << std::endl;
                        }
                    }

                    if (vBuf[i].canLayeredTri2(layer)) {
                        s16 triTwo[6] = { vBuf[i].getLayeredVtxIndex(layer), vBuf[i].getLayeredVtxIndex(layer), vBuf[i].getLayeredVtxIndex(layer),
                                          vBuf[i].getLayeredVtxIndex(layer), vBuf[i].getLayeredVtxIndex(layer), vBuf[i].getLayeredVtxIndex(layer) };
                        if (WriteTri(triTwo, 6)) {
                            gfxOut << "gsSP2Triangles "
                                   << triTwo[0] << ", "
                                   << triTwo[1] << ", "
                                   << triTwo[2] << ", 0x00, "
                                   << triTwo[3] << ", "
                                   << triTwo[4] << ", "
                                   << triTwo[5] << ", 0x00"
                                   << std::endl;
                        }
                    } else {
                        s16 triOne[3] = { vBuf[i].getLayeredVtxIndex(layer), vBuf[i].getLayeredVtxIndex(layer), vBuf[i].getLayeredVtxIndex(layer) };
                        if (WriteTri(triOne, 3)) {
                            gfxOut << "gsSP1Triangle "
                                   << triOne[0] << ", "
                                   << triOne[1] << ", "
                                   << triOne[2] << ", 0x00" << std::endl;
                        }
                    }
                }
            }
        }
        bool clearOring = false;
        /* Reset display list settings */
        gfxOut << "gsSPTexture -1, -1, 0, 0, 0" << std::endl
               << "gsDPPipeSync" << std::endl
               << "gsDPSetCombineMode1Cycle G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_SHADE, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE" << std::endl
               << "gsSPSetGeometryMode G_LIGHTING" << std::endl
               << "gsDPSetTextureLUT G_TT_NONE" << std::endl;

        if (twoCycle) { /* Return back to 1 cycle */
            gfxOut << "gsDPSetCycleType G_CYC_1CYCLE" << std::endl;
        }

        if (fog) {
            gfxOut << "gsDPSetRenderMode G_RM_AA_ZB_OPA_SURF, G_RM_NOOP2" << std::endl
                   << "gsSPClearGeometryMode G_FOG" << std::endl;
        }

        /* Disable group tags */

        if (oldGeo[ENVMAP]) {
            gfxOut << "gsSPClearGeometryMode G_TEXTURE_GEN";
            clearOring = true;
        }

        if (oldGeo[LIN_ENVMAP]) {
            if (!clearOring) {
                gfxOut << "gsSPClearGeometryMode G_TEXTURE_GEN_LINEAR";
            } else {
                gfxOut << " | G_TEXTURE_GEN_LINEAR";
            }
            clearOring = true;
        }

        if (clearOring) {
            gfxOut << std::endl;
        }

        if (oldGeo[BACKFACE]) {
            gfxOut << "gsSPClearGeometryMode G_CULL_BACK" << std::endl;
        }

        gfxOut << "gsSPEndDisplayList" << std::endl;
    }
};
