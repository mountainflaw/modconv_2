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
#pragma once
#include "modconv.hxx"

/*
 * Display list class
 * Used to build the up to 8 different types of displaylists.
 */

#define MAT_NOT_LAYER -2
#define PROPERTIES 5
enum Properties { ENV, COMBINER, TEXFILTER, TEXLOAD, TEXSCALE };
extern u32 geometryState;
extern const std::string dlTypes[8];

class DisplayList {
    private:
    u8 layer = 1;
    bool twoCycle = false;
    bool first = true;
    std::string fOut;

    const std::string renderModes[8][2] = {
        {"G_RM_ZB_OPA_SURF", "G_RM_ZB_OPA_SURF2"},
        {"G_RM_AA_ZB_OPA_SURF", "G_RM_AA_ZB_OPA_SURF2"},
        {"G_RM_AA_ZB_OPA_DECAL", "G_RM_AA_ZB_OPA_DECAL2"},
        {"G_RM_AA_ZB_OPA_INTER", "G_RM_AA_ZB_OPA_INTER2"},
        {"G_RM_AA_ZB_TEX_EDGE", "G_RM_AA_ZB_TEX_EDGE2"},
        {"G_RM_AA_ZB_XLU_SURF", "G_RM_AA_ZB_XLU_SURF2"},
        {"G_RM_AA_ZB_XLU_DECAL", "G_RM_AA_ZB_XLU_DECAL2"},
        {"G_RM_AA_ZB_XLU_INTER", "G_RM_AA_ZB_XLU_INTER2"}
    };

    INLINE bool WriteTri(s16 tri[], u8 size) {
        for (u8 i = 0; i < size; i++) {
            if (tri[i] == -1) {
                return false;
            }
        }
        return true;
    }

    std::string GetRenderMode() {
        return dl_command("gsDPSetRenderMode", renderModes[layer][CYCLE1] + ", " + renderModes[layer][CYCLE2]) + "\n";
    }

    std::string GetRenderModeFog() {
        return dl_command("gsDPSetRenderMode", "G_RM_FOG_SHADE_A, " + renderModes[layer][CYCLE2]) + "\n";
    }

    /** Returns delta material settings. */
    std::string store[PROPERTIES];
    std::string GetMaterial(Material* mat, u16 currMat, const bool first) {
        bool properties[PROPERTIES] = {0};
        std::string lights = "";

        if (first) {
            store[ENV] = mat[currMat].getEnvColor();
            store[COMBINER] = mat[currMat].getSetCombine(layer, twoCycle);
            store[TEXFILTER] = mat[currMat].getTextureFilter();
            store[TEXLOAD] = mat[currMat].getTextureLoad();
            store[TEXSCALE] = mat[currMat].getTextureScaling();

            if (mat[currMat].getLighting(&geometryState)) {
                lights  = dl_command("gsSPNumLights", "NUMLIGHTS_1") + "\n";
                lights += dl_command_ref("gsSPLight", get_filename(fOut) + "_diffuse_light, 1") + "\n";
                lights += dl_command_ref("gsSPLight", get_filename(fOut) + "_ambient_light, 2") + "\n";
            }

            return dl_command("gsDPPipeSync") + "\n" + mat[currMat].getGeometryMode(&geometryState) + lights + store[ENV] + store[COMBINER] + store[TEXFILTER] + store[TEXLOAD] + store[TEXSCALE] + dl_command("gsDPTileSync") + "\n";
        }

        if (mat[currMat].getLighting(&geometryState)) {
            lights  = dl_command("gsSPNumLights", "NUMLIGHTS_1") + "\n";
            lights += dl_command_ref("gsSPLight", get_filename(fOut) + "_diffuse_light, 1") + "\n";
            lights += dl_command_ref("gsSPLight", get_filename(fOut) + "_ambient_light, 2") + "\n";
        }

        std::string write[PROPERTIES];
        std::string load[PROPERTIES] = {
            mat[currMat].getEnvColor(),
            mat[currMat].getSetCombine(layer, twoCycle),
            mat[currMat].getTextureFilter(),
            mat[currMat].getTextureLoad(),
            mat[currMat].getTextureScaling()
        };

        for (u16 i = 0; i < PROPERTIES; i++) {
            if (store[i] != load[i]) {
                if (load[i] == "") { /* sometimes material properties return nothing if they're redundant/not needed */
                    write[i] = "";
                    continue;
                }

                store[i] = load[i];
                write[i] = load[i];
                properties[i] = true;
            } else {
                write[i] = "";
            }
        }

        std::string ret = "";

        if (properties[ENV] || properties[COMBINER] || properties[TEXLOAD] || properties[TEXSCALE]) {
            ret += dl_command("gsDPPipeSync") + "\n";
        }

        ret += mat[currMat].getGeometryMode(&geometryState) + lights + write[ENV] + write[COMBINER] + write[TEXFILTER] + write[TEXLOAD] + write[TEXSCALE];

        if (properties[TEXLOAD] || properties[TEXSCALE]) {
            ret += dl_command("gsDPTileSync") + "\n";
        }

        return ret;
    }

    INLINE std::string is_const() {
        if (!glabel) {
            return "const ";
        }
        return "";
    }

    public:
    INLINE void setLayer(u8 l) { layer = l; }

    void writeDisplayList(const std::string &fileOut, VertexBuffer *vBuf, u16 vBuffers, Material* mat) {
        std::fstream gfxOut;
        s16 currMat = -1; /* force update at start*/

        fOut = fileOut;

        gfxOut.open(fileOut + "/model.inc.c", std::ofstream::out | std::ofstream::app);
        gfxOut << std::endl << "/* Render order: " << (u16)layer << " */";
        gfxOut << std::endl << is_const() << "Gfx " << fileOut + "_dl_" + dlTypes[layer] << "[] = {" << std::endl;

        gfxOut << dl_command("gsSPClearGeometryMode", "G_LIGHTING") << std::endl;

        if (twoCycle || fog) {
            gfxOut << "gsDPSetCycleType G_CYC_2CYCLE" << std::endl;
        }

        if (layer > 3) {
            gfxOut << dl_command("gsDPSetDepthSource", "G_ZS_PIXEL") << std::endl;
        }

        if (fog) {
            twoCycle = true; /* so G_CYC_2CYCLE is disabled */
            gfxOut << GetRenderModeFog()
                   << dl_command("gsSPSetGeometryMode", "G_FOG") << std::endl
                   << dl_command("gsSPFogPosition", std::to_string(fogSettings[4]) + ", " + std::to_string(fogSettings[5])) << std::endl
                   << dl_command("gsDPSetFogColor", std::to_string(fogSettings[0]) + ", " + std::to_string(fogSettings[1]) + ", " + std::to_string(fogSettings[2]) + ", " + std::to_string(fogSettings[3]))
                   << std::endl;
        }

        if (gGeneric && !fog) {
                gfxOut << GetRenderMode();
        }

        for (u16 i = 0; i < vBuffers; i++) {
            vBuf[i].vtxCount = 0; /* reset this from the last layer */
            if (vBuf[i].hasLayer(layer)) { /* don't load what we don't need */
                if (vBuf[i].getLayeredVtxMat(layer) != currMat && vBuf[i].getLayeredVtxMat(layer) != MAT_NOT_LAYER) { /* load before vtx load if possible */
                    currMat = vBuf[i].getLayeredVtxMat(layer);

                    gfxOut << "    /* " << mat[currMat].getName() << " */" << std::endl;

                    gfxOut << GetMaterial(mat, currMat, first);
                    first = false;
                }

                gfxOut << dl_command("gsSPVertex", get_filename(fileOut) + "_vertex_" + std::to_string(i) + ", " + std::to_string(vBuf[i].loadSize) + ", 0") << std::endl;

                while (!vBuf[i].isBufferComplete()) {
                    if (vBuf[i].getVtxMat() != currMat && vBuf[i].getLayeredVtxMat(layer) != MAT_NOT_LAYER) {
                        currMat = vBuf[i].getLayeredVtxMat(layer);
                        bool resetVtxCache = mat[currMat].getLighting(&geometryState);

                        gfxOut << "    /* " << mat[currMat].getName() << " */" << std::endl;
                        gfxOut << GetMaterial(mat, currMat, first);
                        first = false;

                        if (resetVtxCache) {
                            gfxOut << dl_command("gsSPVertex", get_filename(fileOut) + "_vertex_" + std::to_string(i) + ", " + std::to_string(vBuf[i].loadSize) + ", 0") << std::endl;
                        }
                    }

                    if (vBuf[i].canLayeredTri2(layer)) {
                        s16 triTwo[6] = { vBuf[i].getLayeredVtxIndex(layer), vBuf[i].getLayeredVtxIndex(layer), vBuf[i].getLayeredVtxIndex(layer),
                                          vBuf[i].getLayeredVtxIndex(layer), vBuf[i].getLayeredVtxIndex(layer), vBuf[i].getLayeredVtxIndex(layer) };
                        if (WriteTri(triTwo, 6)) {
                            gfxOut << dl_command("gsSP2Triangles",
                                    std::to_string(triTwo[0]) + ", " + std::to_string(triTwo[1]) + ", " + std::to_string(triTwo[2]) + ", 0x00, " + std::to_string(triTwo[3]) + ", " + std::to_string(triTwo[4]) + ", " + std::to_string(triTwo[5]) + ", 0x00") << std::endl;
                        }
                    } else {
                        s16 triOne[3] = { vBuf[i].getLayeredVtxIndex(layer), vBuf[i].getLayeredVtxIndex(layer), vBuf[i].getLayeredVtxIndex(layer) };
                        if (WriteTri(triOne, 3)) {
                            gfxOut << dl_command("gsSP1Triangle", std::to_string(triOne[0]) + ", " + std::to_string(triOne[1]) + ", " + std::to_string(triOne[2]) + ", 0x00") << std::endl;
                        }
                    }
                }
            }
        }
        //bool clearOring = false;
        /* Reset display list settings */
        gfxOut << dl_command("gsSPTexture", "0xFFFF, 0xFFFF, 0, 0, G_OFF") << std::endl
               << dl_command("gsDPPipeSync") << std::endl
               << dl_command("gsDPSetCombineMode", "G_CC_SHADE, G_CC_SHADE") << std::endl;

        gfxOut << dl_command("gsSPSetGeometryMode", "G_LIGHTING") << std::endl;
        gfxOut << dl_command("gsDPSetTextureLUT", "G_TT_NONE") << std::endl;


        if (store[TEXFILTER] != "G_TF_BILERP\n") {
            std::cout << "texfilter doesn't match :(\n";
        }

        if (twoCycle) { /* Return back to 1 cycle */
            gfxOut << "gsDPSetCycleType G_CYC_1CYCLE" << std::endl;
        }

        if (fog) {
            gfxOut << GetRenderMode() << "gsSPClearGeometryMode G_FOG" << std::endl;
        }

        /* Disable env group tags */

        if (geometryState & GET_GEO(GEO_ENVMAP) && geometryState & GET_GEO(GEO_ENVMAP_L)) {
            gfxOut << dl_command("gsSPClearGeometryMode", "G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR") << std::endl;
        } else if (geometryState & GET_GEO(GEO_ENVMAP)) {
            gfxOut << dl_command("gsSPClearGeometryMode", "G_TEXTURE_GEN") << std::endl;
        } else if (geometryState & GET_GEO(GEO_ENVMAP_L)) {
            gfxOut << dl_command("gsSPClearGeometryMode", "G_TEXTURE_GEN_LINEAR") << std::endl;
        }

        /* Disable backface culling */

        if (geometryState & GET_GEO(GEO_BACKFACE)) {
            gfxOut << dl_command("gsSPSetGeometryMode", "G_CULL_BACK") << std::endl;
        }

        gfxOut << dl_command("gsSPEndDisplayList") << std::endl;

        gfxOut << "};" << std::endl;
    }
};
