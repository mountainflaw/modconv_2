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

#define FORMATS 8

typedef struct {
    std::string path;
    u8 size[2];
    u8 sizeLog2[2];
} Texture;

/** Fast3D material class */
class Material
{
    private:
    Texture tex;
    enum TexType { RGBA16, RGBA32, CI4, CI8, IA4, IA8, I4, I8 };
    const std::string format[FORMATS] = { "rgba16", "rgba32", "ci4", "ci8", "ia4", "ia8", "i4", "i8" };
    std::string name = "DEFAULT MATERIAL (GENERATED NAME)";
    std::string fileOut = "";

    inline std::string NewlineIfTrue(const bool a)
    {
        if (a) { return "\n"; }
        else { return ""; }
    }

    /** Returns texture load string. */
    std::string GetTextureLoad()
    {
        bool tex4b = false;
        std::string ret = "", texLoadType, texLoadSize;
        u8 type = 0; /* RGBA16 just in case */
        for (u8 i = 0; i < FORMATS; i++) {
            if (tex.path.find(format[i]) != std::string::npos) {
                type = i;
                break;
            }
        }

        switch(type) {
            case RGBA32:
            texLoadType = "G_IM_FMT_RGBA, ";
            texLoadSize = "G_IM_SIZ_32b, ";
            break;

            case CI4:
            texLoadType = "G_IM_FMT_CI, ";
            texLoadSize = "G_IM_SIZ_4b, ";
            tex4b = true;
            break;

            case CI8:
            texLoadType = "G_IM_FMT_CI, ";
            texLoadSize = "G_IM_SIZ_8b, ";
            break;

            case IA4:
            texLoadType = "G_IM_FMT_IA, ";
            texLoadSize = "G_IM_SIZ_4b, ";
            tex4b = true;
            break;

            case IA8:
            texLoadType = "G_IM_FMT_IA, ";
            texLoadSize = "G_IM_SIZ_8b, ";
            break;

            case I4:
            texLoadType = "G_IM_FMT_I, ";
            texLoadSize = "G_IM_SIZ_8b, ";
            tex4b = true;
            break;

            case I8:
            texLoadType = "G_IM_FMT_I, ";
            texLoadSize = "G_IM_SIZ_8b, ";
            break;

            case RGBA16:
            default:
            texLoadType = "G_IM_FMT_RGBA, ";
            texLoadSize = "G_IM_SIZ_16b, ";
        }

        if (type == CI4) {
            ret += "gsDPSetTextureLUT G_TT_RGBA16\ngsDPLoadTLUT_pal16 0, " + getFileNameNoExtension() + "_pal\n";
        } else if (type == CI8) {
            ret += "gsDPSetTextureLUT G_TT_RGBA16\ngsDPLoadTLUT_pal256 0, " + getFileNameNoExtension() + "_pal\n";
        } else {
            ret += "gsDPSetTextureLUT G_TT_NONE\n";
        }

        std::string texFlagU = "G_TX_NOMIRROR", texFlagV = "G_TX_NOMIRROR",
        wrapTags[4] = { "#MIRRORU", "#CLAMPU", "#MIRRORV", "#CLAMPV" },
        texFlags[4] = { "G_TX_MIRROR", "G_TX_CLAMP", "G_TX_MIRROR", "G_TX_CLAMP" };

        for (u16 i = 0; i < 4; i++) {
            if (name.find(wrapTags[i]) != std::string::npos) {
                if (i < 2) {
                    texFlagU = texFlags[i];
                } else { /* last two tags are for the v axis */
                    texFlagV = texFlags[i];
                }
            }
        }

        if (tex4b) { /* Thank you SGI, very cool! */
            ret += "gsDPLoadTextureBlock_4b " + getFileNameNoExtension() + ", " + texLoadType + std::to_string(tex.size[AXIS_X]) + ", " + std::to_string(tex.size[AXIS_Y]) + ", 0, G_TX_WRAP | " + texFlagU + ",  G_TX_WRAP | " + texFlagV + ", " + std::to_string(tex.sizeLog2[AXIS_X]) + ", " + std::to_string(tex.sizeLog2[AXIS_Y]) + ", G_TX_NOLOD, G_TX_NOLOD\ngsSPTexture -1, -1, 0, 0, 1\ngsDPTileSync\n";
        } else {
            ret += "gsDPLoadTextureBlock " + getFileNameNoExtension() + ", " + texLoadType + texLoadSize + std::to_string(tex.size[AXIS_X]) + ", " + std::to_string(tex.size[AXIS_Y]) + ", 0, G_TX_WRAP | " + texFlagU + ",  G_TX_WRAP | " + texFlagV + ", " + std::to_string(tex.sizeLog2[AXIS_X]) + ", " + std::to_string(tex.sizeLog2[AXIS_Y]) + ", G_TX_NOLOD, G_TX_NOLOD\ngsSPTexture -1, -1, 0, 0, 1\ngsDPTileSync\n";
        }
        return ret;
    }

    /** Returns combiner settings. */
    std::string GetFuckingFrauber(const u8 layer, const bool twoCycle)
    {
        if (twoCycle) { /* Shoutouts to SGI once again, for making the RDP incredibly annoying. */
            if (name.find("#DIFFUSE") != std::string::npos) {
                goto untextured_2cycle;
            } else if (textured) {
                if (layer > 1) { /* lazy way to fix transparency */
                    return "gsDPSetCombineMode G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_TEXEL0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_TEXEL0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_COMBINED, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_COMBINED\n";
                } else {
                    return "gsDPSetCombineMode G_CCMUX_TEXEL0, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE, G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_COMBINED, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_COMBINED\n";
                }
            } else {
                untextured_2cycle:
                return "gsDPSetCombineMode G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_SHADE, G_ACMUX_1, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE, G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_COMBINED, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_COMBINED\n";
            }
        }

        else { /* 1 cycle combiners */
            if (name.find("#DIFFUSE") != std::string::npos) {
                goto untextured_1cycle;
            } else if (textured) {
                if (layer > 1) {
                    return "gsDPSetCombineMode1Cycle G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_TEXEL0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_TEXEL0\n";
                } else {
                    return "gsDPSetCombineMode1Cycle G_CCMUX_TEXEL0, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE\n";
                }
            } else {
                    untextured_1cycle:
                    return "gsDPSetCombineMode1Cycle G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_SHADE, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE\n";
                }
        }
    }

#define GROUP_TAGS 5
bool ourGeo[GROUP_TAGS] = {0x00};
std::string groupTags[GROUP_TAGS] = { "#ENVMAP", "#LIN_ENVMAP", "#LIGHTING", "#SHADE", "#BACKFACE" },
    geoModes[GROUP_TAGS] = { "G_TEXTURE_GEN", "G_TEXTURE_GEN_LINEAR", "G_LIGHTING", "G_SHADE", "G_CULL_BACK"};

    std::string GetGeometryMode(bool* oldGeo)
    {
        std::string setRet = "", clearRet = "";
        bool clearOring = false, setOring = false;
        for (u8 i = 0; i < GROUP_TAGS; i++) {
            if (name.find(groupTags[i]) != std::string::npos) {
                ourGeo[i] = true;
            }
        }

        for (u8 i = 0; i < GROUP_TAGS; i++) {
            if ((ourGeo[i] && !oldGeo[i]) || (!oldGeo[i] && ourGeo[i])) { /* set */
                if (setOring && i != BACKFACE) {
                    setRet += " | " + geoModes[i];
                } else if (!setOring && i != BACKFACE) {
                    setOring = true;
                    setRet = "gsSPSetGeometryMode " + geoModes[i];
                }

                /* !! The #BACKFACE group tag DISABLES backface culling !!*/

                else if (clearOring && i == BACKFACE) {
                    clearRet += " | " + geoModes[i];
                } else if (!clearOring && i == BACKFACE) {
                    clearOring = true;
                    clearRet = "gsSPClearGeometryMode " + geoModes[i];
                }
            }

            if ((!ourGeo[i] && oldGeo[i]) || (oldGeo[i] && !ourGeo[i])) { /* clear */
                if (clearOring && i != BACKFACE) {
                    clearRet += " | " + geoModes[i];
                } else if (!clearOring && i != BACKFACE) {
                    clearOring = true;
                    clearRet = "gsSPClearGeometryMode " + geoModes[i];
                }

                /* !! The #BACKFACE group tag DISABLES backface culling !!*/

                else if (setOring && i == BACKFACE) {
                    setRet += " | " + geoModes[i];
                } else if (!setOring && i == BACKFACE) {
                    setOring = true;
                    clearRet = "gsSPSetGeometryMode " + geoModes[i];
                }
            }
        }

        std::string lights = "";

        if (ourGeo[LIGHTING]) {

            lights = "gsSPNumLights NUMLIGHTS_1\n";
            lights += "gsSPLight " + get_filename(fileOut) + "_diffuse_light, 1\n";
            lights += "gsSPLight " + get_filename(fileOut) + "_ambient_light, 2\n";
        }

        /* copy over current geo to old geo */
        for (u8 i = 0; i < GROUP_TAGS; i++) {
            if (ourGeo[i]) {
                oldGeo[i] = ourGeo[i];
            }
        }

        return setRet + NewlineIfTrue(setOring) + clearRet + NewlineIfTrue(clearOring) + lights;
    }

    public:
    enum GeoModes { ENVMAP, LIN_ENVMAP, LIGHTING, SHADE, BACKFACE};
    bool useless = false, textured = false;
    void setName(const std::string &n) { name = n; }
    void setFile(const std::string &f) { fileOut = f; }

    /**
     * Returns true if lighting will be enabled.
     * Used to fix weird colors caused by
     * enabling lighting in some situations.
     */

    bool getLighting(const bool* oldGeo) { return !oldGeo[LIGHTING] && ourGeo[LIGHTING]; }

    void setPath(const std::string &p)
    {
        tex.path = p;
        tex.size[AXIS_X] = get_dimension(AXIS_X, p);
        tex.size[AXIS_Y] = get_dimension(AXIS_Y, p);

        tex.sizeLog2[AXIS_X] = log2(tex.size[AXIS_X]);
        tex.sizeLog2[AXIS_Y] = log2(tex.size[AXIS_Y]);
    }

    s16 getDimension(u8 axis) { return tex.size[axis]; }
    std::string getPath() { return tex.path; }
    std::string getName() { return name; }


    std::string getFileNameNoExtension()
    {
        std::string toReturn = get_filename(tex.path);
        return toReturn.substr(0, toReturn.length() - 4); /* <- Hack */
    }

    /** Configures the Material object. (Unused for now) */
    void setMaterial(std::string n, std::string p) {}

    /** Returns the exact F3D settings that represent this material. */
    std::string getMaterial(bool* oldGeo, const u8 layer, const bool twoCycle)
    {
        std::string ret;
        ret += GetGeometryMode(oldGeo);
        if (textured) {
            ret += GetFuckingFrauber(layer, twoCycle) + GetTextureLoad();
        } else { /* no texture found when setting up */
            ret += GetFuckingFrauber(layer, twoCycle) + "gsDPSetPrimColor 0x00, 0x00, 128, 128, 128, 0xFF\n";
        }
        return ret;
    }
};
