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
std::string hex_string(u8);

struct Texture {
    std::string path;
    u8 size[2];
    u8 sizeLog2[2];
};

enum Texels { TEXEL0, TEXEL1 };
enum Cycles { CYCLE1, CYCLE2 };

enum GeoFlags {
    GEO_LIGHTING,
    GEO_BACKFACE,
    GEO_ENVMAP,
    GEO_ENVMAP_L
};

#define CURRENT_GEO (1 << i)
#define GET_GEO(x)  (1 << x)

/** Fast3D material class */
class Material {
    private:
    Texture tex;
    enum TexType { RGBA16, RGBA32, CI4, CI8, IA4, IA8, I4, I8 };
    std::string name = "DEFAULT MATERIAL (GENERATED NAME)";
    std::string fileOut = "";

    INLINE std::string NewlineIfTrue(const bool a) {
        if (a) { return "\n"; }
        else { return ""; }
    }

#define GROUP_TAGS 4
u32 geometryFlags = 0;
std::string groupTags[GROUP_TAGS] = { "#LIGHTING", "#BACKFACE", "#ENVMAP", "#LIN_ENVMAP" },
            geoModes[GROUP_TAGS]  = { "G_LIGHTING", "G_CULL_BACK", "G_TEXTURE_GEN", "G_TEXTURE_GEN_LINEAR" };

    public:
    bool useless = false, textured = false;
    u8 diffuse[3];
    INLINE void setName(const std::string &n) { name = n; }
    INLINE void setFile(const std::string &f) { fileOut = f; }
    INLINE void setIndex(const u16 i) { index = i; }
    u16 index;

    /**
     * Returns true if lighting will be enabled.
     * Used to fix weird colors caused by
     * enabling lighting in some situations.
     */

    bool getLighting(u32* g) {
        return !(*g & (1 << GEO_LIGHTING)) && (name.find(groupTags[GEO_LIGHTING]) != std::string::npos);
    }

    std::string getEnvColor() {
        if (name.find("#DIFFUSE") != std::string::npos) {
            return dl_command("gsDPSetEnvColor", "0x00, 0x00 " + hex_string(diffuse[C_RED]) + ", " + hex_string(diffuse[C_GRN]) + ", " + hex_string(diffuse[C_BLU])) + "\n";
        }
        return "";
    }

    /** Returns combiner settings (new) */
    std::string getSetCombine(const u8 layer, const bool twoCycle) {
        std::string combiner[2];

        /* combiner override */
        if (name.find("$") != std::string::npos) {
            u16 pos = 0;
            pos = name.find("$") + 1;
            for (u16 j = pos; j < name.length(); j++) {
                if (name[j] == ' ') {
                    break;
                }
                combiner[CYCLE1] = name.substr(pos, (j - pos) + 1);
            }
            pos = 0;
            if (name.find("%") != std::string::npos) {
                pos = name.find("%") + 1;
                for (u16 j = pos; j < name.length(); j++) {
                    if (name[j] == ' ') {
                        break;
                    }
                    combiner[CYCLE2] = name.substr(pos, (j - pos) + 1);
                }
            } else { /* user did not specify cycle 2 */
                combiner[CYCLE2] = combiner[CYCLE1];
            }

            return dl_command("gsDPSetCombine", combiner[CYCLE1] + ", " + combiner[CYCLE2]) + "\n";
        }

        if (name.find("#DIFFUSE") != std::string::npos) { /* Custom fallback material. */
            std::string ret;
            if (twoCycle) {
                return dl_command("gsDPSetCombineMode", "G_CCMUX_SHADE, G_CCMUX_0, G_CCMUX_ENVIRONMENT, G_CCMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE, G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_COMBINED, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_COMBINED") + "\n";
            } else { /* 1 cycle */
                return dl_command("gsDPSetCombineMode", "G_CCMUX_SHADE, G_CCMUX_0, G_CCMUX_ENVIRONMENT, G_CCMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE, G_CCMUX_SHADE, G_CCMUX_0, G_CCMUX_ENVIRONMENT, G_CCMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE") + "\n";
            }
        }

        if (twoCycle && textured) { /* 2 cycle */
            switch (layer) {
                case 0: /* opaque */
                case 1:
                case 2:
                case 3:
                return dl_command("gsDPSetCombineMode", "G_CCMUX_TEXEL0, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE, G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_COMBINED, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_COMBINED");
                break;

                case 4: /* alpha */
                case 5: /* transparent */
                case 6:
                case 7:
                return dl_command("gsDPSetCombineMode", "G_CCMUX_TEXEL0, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0, G_ACMUX_TEXEL0, G_ACMUX_0, G_ACMUX_SHADE, G_ACMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_COMBINED, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_COMBINED");
                break;
            }
        } else if (!twoCycle && textured) { /* 1 cycle */
            switch (layer) {
                case 0: /* opaque */
                case 1:
                case 2:
                case 3:
                return dl_command("gsDPSetCombineMode", "G_CCMUX_TEXEL0, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE, G_CCMUX_TEXEL0, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE");
                break;

                case 4: /* alpha */
                case 5: /* transparent */
                case 6:
                case 7:
                return dl_command("gsDPSetCombineMode", "G_CCMUX_TEXEL0, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0, G_ACMUX_TEXEL0, G_ACMUX_0, G_ACMUX_SHADE, G_ACMUX_0, G_CCMUX_TEXEL0, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0, G_ACMUX_TEXEL0, G_ACMUX_0, G_ACMUX_SHADE, G_ACMUX_0");
                break;
            }
        }
        /* diffuse fallback */
        if (twoCycle) {
            return dl_command("gsDPSetCombineMode", "G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_SHADE, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE, G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_COMBINED, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_COMBINED") + "\n";
        } else { /* 1 cycle */
            return dl_command("gsDPSetCombineMode", "G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_SHADE, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE, G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_SHADE, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE") + "\n";
        }
    }

    /** Returns texture load string. */
    std::string getTextureLoad() {
        if (!textured) {
            return "";
        }

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
            ret += dl_command("gsDPLoadTextureBlock_4b", fileOut + "_texture_" + std::to_string(index) + ", " + texLoadType + std::to_string(tex.size[AXIS_X]) + ", " + std::to_string(tex.size[AXIS_Y]) + ", 0, G_TX_WRAP | " + texFlagU + ",  G_TX_WRAP | " + texFlagV + ", " + std::to_string(tex.sizeLog2[AXIS_X]) + ", " + std::to_string(tex.sizeLog2[AXIS_Y]) + ", G_TX_NOLOD, G_TX_NOLOD") + "\n";
        } else {
            ret += dl_command("gsDPLoadTextureBlock", fileOut + "_texture_" + std::to_string(index) + ", " + texLoadType + texLoadSize + std::to_string(tex.size[AXIS_X]) + ", " + std::to_string(tex.size[AXIS_Y]) + ", 0, G_TX_WRAP | " + texFlagU + ",  G_TX_WRAP | " + texFlagV + ", " + std::to_string(tex.sizeLog2[AXIS_X]) + ", " + std::to_string(tex.sizeLog2[AXIS_Y]) + ", G_TX_NOLOD, G_TX_NOLOD") + "\n";
        }
        return ret;
    }

    std::string getGeometryMode(u32* geometryCurrent) {
        bool setOR = false, clearOR = false;
        std::string setStr = "", clearStr = "";

        for (u8 i = 0; i < GROUP_TAGS; i++) {
            if (name.find(groupTags[i]) != std::string::npos) {
                geometryFlags |= CURRENT_GEO;
                std::cout << (u32)CURRENT_GEO << std::endl;
            }
        }

        for (u8 i = 0; i < GROUP_TAGS; i++) {
            if ((geometryFlags & CURRENT_GEO) && !(*geometryCurrent & CURRENT_GEO)) { /* Set geometry mode */
                if (i == 1) { /* Setting backface culling clears the geometry mode for it */
                    if (!clearOR) {
                        clearStr += geoModes[i];
                        clearOR = true;
                    } else { /* OR */
                        clearStr += " | " + geoModes[i];
                    }
                } else { /* Not backface culling */
                    if (!setOR) {
                        setStr += geoModes[i];
                        setOR = true;
                    } else { /* OR */
                        setStr += " | " + geoModes[i];
                    }
                }
            }

            if (!(geometryFlags & CURRENT_GEO) && (*geometryCurrent & CURRENT_GEO)) { /* Clear geometry mode */
                if (i == 1) { /* Setting backface culling clears the geometry mode for it */
                    if (!setOR) {
                        setStr += geoModes[i];
                        setOR = true;
                    } else { /* OR */
                        setStr += " | " + geoModes[i];
                    }
                } else { /* Not backface culling */
                    if (!clearOR) {
                        clearStr += geoModes[i];
                        clearOR = true;
                    } else { /* OR */
                        clearStr += " | " + geoModes[i];
                    }
                }
            }
        }

        if (setOR) {
            setStr = dl_command("gsSPSetGeometryMode", setStr) + "\n";
        }

        if (clearOR) {
            clearStr = dl_command("gsSPClearGeometryMode", clearStr) + "\n";
        }

        *geometryCurrent = geometryFlags;
        return setStr + clearStr;
    }

    /* Env mapping requires huge scaling */
    std::string getTextureScaling() {
        std::string textureArgs;
        if (name.find("#ENVMAP") != std::string::npos) { /* env mapping */
            /* Nintendo seems to do 62 * size in SM64... */
            return dl_command("gsSPTexture", std::to_string(tex.size[AXIS_X] * 62) + ", " + std::to_string(tex.size[AXIS_Y] * 62) + ", 0, 0, 1") + "\n";
        }
        return dl_command("gsSPTexture", "-1, -1, 0, 0, 1") + "\n";
    }

    std::string getTextureFilter() {
        std::string filter = "G_TF_BILERP";

        if (name.find("#NEAREST") != std::string::npos) {
            filter = "G_TF_POINT";
        }

        if (name.find("#AVERAGE") != std::string::npos) {
            filter = "G_TF_AVERAGE";
        }

        return dl_command("gsDPSetTextureFiler", filter) + "\n";
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
};
