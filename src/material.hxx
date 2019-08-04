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

extern const std::string format[FORMATS];

typedef struct {
    std::string path;
    u8 size[2];
    u8 sizeLog2[2];
} Texture;

/** Fast3D material class */
class Material {
    private:
    Texture tex;
    enum TexType { RGBA16, RGBA32, CI4, CI8, IA4, IA8, I4, I8 };
    std::string name = "DEFAULT MATERIAL (GENERATED NAME)";
    std::string fileOut = "";
    u16 index;

    INLINE std::string NewlineIfTrue(const bool a) {
        if (a) { return "\n"; }
        else { return ""; }
    }

    /** Returns texture load string. */
    std::string GetTextureLoad() {
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
            ret += dl_command("gsDPSetTextureLUT", "G_TT_RGBA16") + "\n" + dl_command("gsDPLoadTLUT_pal16", "0, " + fileOut + "_palette_" + std::to_string(index)) + "\n";
        } else if (type == CI8) {
            ret += dl_command("gsDPSetTextureLUT", "G_TT_RGBA16") + "\n" + dl_command("gsDPLoadTLUT_pal256", "0, " + fileOut + "_palette_" + std::to_string(index)) + "\n";
        } else { /* No palette -- Prevents problems with non-CI formats */
            ret += dl_command("gsDPSetTextureLUT", "G_TT_NONE") + "\n";
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
            ret += dl_command("gsDPLoadTextureBlock_4b", fileOut + "_texture_" + std::to_string(index) + ", " + texLoadType + std::to_string(tex.size[AXIS_X]) + ", " + std::to_string(tex.size[AXIS_Y]) + ", 0, G_TX_WRAP | " + texFlagU + ",  G_TX_WRAP | " + texFlagV + ", " + std::to_string(tex.sizeLog2[AXIS_X]) + ", " + std::to_string(tex.sizeLog2[AXIS_Y]) + ", G_TX_NOLOD, G_TX_NOLOD") + "\n" + dl_command("gsSPTexture", "-1, -1, 0, 0, 1") + "\n" + dl_command("gsDPTileSync", "") + "\n";
        } else {
            ret += dl_command("gsDPLoadTextureBlock", fileOut + "_texture_" + std::to_string(index) + ", " + texLoadType + texLoadSize + std::to_string(tex.size[AXIS_X]) + ", " + std::to_string(tex.size[AXIS_Y]) + ", 0, G_TX_WRAP | " + texFlagU + ",  G_TX_WRAP | " + texFlagV + ", " + std::to_string(tex.sizeLog2[AXIS_X]) + ", " + std::to_string(tex.sizeLog2[AXIS_Y]) + ", G_TX_NOLOD, G_TX_NOLOD") +"\n" + dl_command("gsSPTexture", "-1, -1, 0, 0, 1") + "\n" + dl_command("gsDPTileSync", "") + "\n";
        }
        return ret;
    }

    /** Returns combiner settings (new) */
    std::string GetDrFuckingFrauber(const u8 layer, const bool twoCycle) {
        if (name.find("#DIFFUSE") != std::string::npos) {
            goto untextured;
        }

        if (twoCycle && textured) { /* 2 cycle */
            switch (layer) {
                case 0: /* opaque */
                case 1:
                case 2:
                case 3:
                return dl_command("gsDPSetCombineMode", "G_CCMUX_TEXEL0, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE, G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_COMBINED, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_COMBINED") + "\n";
                break;

                case 4: /* alpha */
                return dl_command("gsDPSetCombineMode", "G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_TEXEL0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_TEXEL0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_COMBINED, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_COMBINED") + "\n";
                break;

                case 5: /* transparent */
                case 6:
                case 7:
                return dl_command("gsDPSetCombineMode", "G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_TEXEL0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_TEXEL0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_COMBINED, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_COMBINED") + "\n";
                break;
            }
        } else if (!twoCycle && textured) { /* 1 cycle */
            switch (layer) {
                case 0: /* opaque */
                case 1:
                case 2:
                case 3:
                return dl_command("gsDPSetCombineMode", "G_CCMUX_TEXEL0, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE, G_CCMUX_TEXEL0, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE") + "\n";
                break;

                case 4: /* alpha */
                return dl_command("gsDPSetCombineMode", "G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_TEXEL0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_TEXEL0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_TEXEL0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_TEXEL0") + "\n";
                break;

                case 5: /* transparent */
                case 6:
                case 7:
                return dl_command("gsDPSetCombineMode", "G_CCMUX_TEXEL0, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0, G_ACMUX_TEXEL0, G_ACMUX_0, G_ACMUX_SHADE, G_ACMUX_0, G_CCMUX_TEXEL0, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0, G_ACMUX_TEXEL0, G_ACMUX_0, G_ACMUX_SHADE, G_ACMUX_0") + "\n";
                break;
            }
        }
        /* diffuse fallback */
        untextured:
        if (twoCycle) {
            return dl_command("gsDPSetCombineMode", "G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_SHADE, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE, G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_COMBINED, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_COMBINED") + "\n";
        } else { /* 1 cycle */
            return dl_command("gsDPSetCombineMode", "G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_SHADE, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE, G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_SHADE, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE") + "\n";
        }
    }

#define GROUP_TAGS 5
bool ourGeo[GROUP_TAGS] = {0x00};
std::string groupTags[GROUP_TAGS] = { "#ENVMAP", "#LIN_ENVMAP", "#LIGHTING", "#SHADE", "#BACKFACE" },
    geoModes[GROUP_TAGS] = { "G_TEXTURE_GEN", "G_TEXTURE_GEN_LINEAR", "G_LIGHTING", "G_SHADE", "G_CULL_BACK" };

    std::string GetGeometryMode(bool* oldGeo) {
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
                    setOring = true;
                    setRet += " | " + geoModes[i];
                } else if (!setOring && i != BACKFACE) {
                    setOring = true;
                    setRet = "gsSPSetGeometryMode " + geoModes[i];
                }

                /* !! The #BACKFACE group tag DISABLES backface culling !!*/

                else if (clearOring && i == BACKFACE) {
                    clearOring = true;
                    clearRet += " | " + geoModes[i];
                } else if (!clearOring && i == BACKFACE) {
                    clearOring = true;
                    clearRet = "gsSPClearGeometryMode " + geoModes[i];
                }
            }

            if ((!ourGeo[i] && oldGeo[i]) || (oldGeo[i] && !ourGeo[i])) { /* clear */
                if (clearOring && i != BACKFACE) {
                    clearOring = true;
                    clearRet += " | " + geoModes[i];
                } else if (!clearOring && i != BACKFACE) {
                    clearOring = true;
                    clearRet = "gsSPClearGeometryMode " + geoModes[i];
                }

                /* !! The #BACKFACE group tag DISABLES backface culling !!*/

                else if (setOring && i == BACKFACE) {
                    setOring = true;
                    setRet += " | " + geoModes[i];
                } else if (!setOring && i == BACKFACE) {
                    setOring = true;
                    setRet = "gsSPSetGeometryMode " + geoModes[i];
                }
            }
        }

        std::string lights = "";

        if (ourGeo[LIGHTING]) {

            lights = dl_command("gsSPNumLights", "NUMLIGHTS_1") + "\n";
            lights += dl_command("gsSPLight", get_filename(fileOut) + "_diffuse_light, 1") + "\n";
            lights += dl_command("gsSPLight", get_filename(fileOut) + "_ambient_light, 2") + "\n";
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
    INLINE void setName(const std::string &n) { name = n; }
    INLINE void setFile(const std::string &f) { fileOut = f; }
    INLINE void setIndex(const u16 i) { index = i; }
    INLINE u16 getIndex() { return index; }

    /**
     * Returns true if lighting will be enabled.
     * Used to fix weird colors caused by
     * enabling lighting in some situations.
     */

    bool getLighting(const bool* oldGeo) {
        for (u8 i = 0; i < GROUP_TAGS; i++) {
            if (name.find(groupTags[i]) != std::string::npos) {
                ourGeo[i] = true;
            }
        }

        return !oldGeo[LIGHTING] && ourGeo[LIGHTING];
    }

    void setPath(const std::string &p) {
        tex.path = p;
        tex.size[AXIS_X] = get_dimension(AXIS_X, p);
        tex.size[AXIS_Y] = get_dimension(AXIS_Y, p);

        tex.sizeLog2[AXIS_X] = log2(tex.size[AXIS_X]);
        tex.sizeLog2[AXIS_Y] = log2(tex.size[AXIS_Y]);
    }

    s16 getDimension(u8 axis) { return tex.size[axis]; }
    std::string getPath() { return tex.path; }
    std::string getName() { return name; }


    std::string getFileNameNoExtension() {
        std::string toReturn = get_filename(tex.path);
        return toReturn.substr(0, toReturn.length() - 4); /* <- Hack */
    }

    /** Configures the Material object. (Unused for now) */
    void setMaterial(std::string n, std::string p) {}

    /** Returns the exact F3D settings that represent this material. */
    std::string getMaterial(bool* oldGeo, const u8 layer, const bool twoCycle) {
        std::string ret;
        ret += GetGeometryMode(oldGeo);
        if (textured) {
            ret += GetDrFuckingFrauber(layer, twoCycle) + GetTextureLoad();
        } else { /* no texture found when setting up */
            ret += GetDrFuckingFrauber(layer, twoCycle);
        }
        return ret;
    }
};
