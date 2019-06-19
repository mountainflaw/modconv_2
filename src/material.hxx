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
    std::string name = "DEFAULT MATERIAL";

    /** Returns texture load string. */
    std::string GetTextureLoad()
    {
        std::string ret = " ", texLoadType, texLoadSize;
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
            break;

            case CI8:
            texLoadType = "G_IM_FMT_CI, ";
            texLoadSize = "G_IM_SIZ_8b, ";
            break;

            case IA4:
            texLoadType = "G_IM_FMT_IA, ";
            texLoadSize = "G_IM_SIZ_4b, ";
            break;

            case IA8:
            texLoadType = "G_IM_FMT_IA, ";
            texLoadSize = "G_IM_SIZ_8b, ";
            break;

            case I4:
            texLoadType = "G_IM_FMT_I, ";
            texLoadSize = "G_IM_SIZ_8b, ";
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
        }

        else if (type == CI8) {
            ret += "gsDPSetTextureLUT G_TT_RGBA16\ngsDPLoadTLUT_pal256 0, " + getFileNameNoExtension() + "_pal\n";
        }

        else {
            ret += "gsDPSetTextureLUT G_TT_NONE\n";
        }

        std::string texFlag = "G_TX_NOMIRROR",
        wrapTags[2] = { "#MIRROR", "#CLAMP" },
        texFlags[2] = { "G_TX_MIRROR", "G_TX_CLAMP" };

        for (u16 i = 0; i < 2; i ++) {
            if (name.find(wrapTags[i]) != std::string::npos) {
                texFlag = texFlags[i];
                break;
            }
        }
        ret += "gsDPLoadTextureBlock " + getFileNameNoExtension() + ", " + texLoadType + texLoadSize + std::to_string(tex.size[AXIS_X]) + ", " + std::to_string(tex.size[AXIS_Y]) + ", 0, G_TX_WRAP | " + texFlag + ",  G_TX_WRAP | " + texFlag + ", " + std::to_string(tex.sizeLog2[AXIS_X]) + ", " + std::to_string(tex.sizeLog2[AXIS_Y]) + ", G_TX_NOLOD, G_TX_NOLOD\ngsSPTexture -1, -1, 0, 0, 1\ngsDPTileSync\n";
        return ret;
    }

    /** Returns combiner settings. */
    std::string GetFuckingFrauber() /** TODO: Get this to support fog, semitransparency, and both if possible. */
    {
        if (textured) {
            return "gsDPSetCombineMode1Cycle G_CCMUX_TEXEL0, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE\n";
        }

        else {
            return "gsDPSetCombineMode1Cycle G_CCMUX_PRIMITIVE, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE\n";
        }
    }

#define GROUP_TAGS 5
enum GeoModes { ENVMAP, LIN_ENVMAP, LIGHTING, SHADE, BACKFACE};
bool ourGeo[GROUP_TAGS] = {0x00};
std::string groupTags[GROUP_TAGS] = { "#ENVMAP", "#LIN_ENVMAP", "#LIGHTING", "#SHADE", "#BACKFACE" },
    geoModes[GROUP_TAGS] = { "G_TEXTURE_GEN", "G_TEXTURE_GEN_LINEAR", "G_LIGHTING", "G_SHADE", "G_CULL_BACK"};

    std::string GetGeometryMode(bool oldGeo[GROUP_TAGS])
    {
        std::string setRet = "", clearRet = "";
        bool setOring = false, clearOring = false;
        for (u8 i = 0; i < GROUP_TAGS; i++) {
            if (name.find(groupTags[i]) != std::string::npos) {
                ourGeo[i] = true;
                std::cout << "enabling geo mode " << i << std::endl;
                std::cout << name << std::endl;
            }

            else {
                std::cout << "not enabling geo mode " << i << std::endl;
                std::cout << name << std::endl;
            }
        }

        for (u8 i = 0; i < GROUP_TAGS; i++) {
            if (ourGeo[i] && !oldGeo[i]) { /* set */
                if (setOring && i != BACKFACE) {
                    setRet += " | " + geoModes[i];
                }

                else if (!setOring && i != BACKFACE) {
                    setOring = true;
                    setRet = "gsSPSetGeometryMode " + geoModes[i];
                }

                /* !! The #BACKFACE group tag DISABLES backface culling !!*/

                else if (clearOring && i == BACKFACE) {
                    clearRet += " | " + geoModes[i];
                }

                else if (!clearOring && i == BACKFACE) {
                    clearOring = true;
                    clearRet = "gsSPClearGeometryMode " + geoModes[i];
                }
            }

            if (!ourGeo[i] && oldGeo[i]) { /* clear */
                if (clearOring && i != BACKFACE) {
                    clearRet += " | " + geoModes[i];
                }

                else if (!clearOring && i != BACKFACE) {
                    clearOring = true;
                    clearRet = "gsSPClearGeometryMode " + geoModes[i];
                }

                /* !! The #BACKFACE group tag DISABLES backface culling !!*/

                else if (setOring && i == BACKFACE) {
                    setRet += " | " + geoModes[i];
                }

                else if (!setOring && i == BACKFACE) {
                    setOring = true;
                    clearRet = "gsSPSetGeometryMode " + geoModes[i];
                }
            }
        }

        return setRet + clearRet + "\n";
    }

    public:
    bool useless = false, textured = false;
    void setName(const std::string &n) { name = n; }
    void setPath(const std::string &p)
    {
        tex.path = p;
        tex.size[AXIS_X] = get_dimension(AXIS_X, p);
        tex.size[AXIS_Y] = get_dimension(AXIS_Y, p);

        tex.sizeLog2[AXIS_X] = log2(tex.size[AXIS_X]);
        tex.sizeLog2[AXIS_Y] = log2(tex.size[AXIS_Y]);
    }

    std::string getPath() { return tex.path; }
    std::string getName() { return name; }


    std::string getFileNameNoExtension()
    {
        std::string toReturn = get_filename(tex.path);
        return toReturn.substr(0, toReturn.length() - 4); /* <- Hack */
    }

    /** Configures the Material object. (Unused for now) */
    void setMaterial(std::string n, std::string p) {}

    /** Returns the incbins. */
    std::string d;
    /** Returns the exact F3D settings that represent this material. */
    std::string getMaterial(bool oldGeo[GROUP_TAGS])
    {
        std::string ret;
        ret += GetGeometryMode(oldGeo);
        if (textured) {
            ret += GetFuckingFrauber() + GetTextureLoad();
        }

        else { /* no texture found when setting up */
            ret += GetFuckingFrauber() + "gsDPSetPrimColor 0x00, 0x00, 128, 128, 128, 0xFF\n";
        }
        return ret;
    }
};
