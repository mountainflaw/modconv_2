#define FORMATS 4

/** Fast3D material class */
class Material
{
    private:
    enum TexType { RGBA16, RGBA32, CI4, CI8 };
    const std::string formats[FORMATS] = { "rgba16", "rgba32", "ci4", "ci8" };
    std::string path, name;

    std::string GetTextureLoad()
    {
        std::string ret;
        u8 type = 0; /* RGBA16 just in case */
        for (u8 i = 0; i < FORMATS; i++) {
            if (path.find(formats[i]) != std::string::npos) {
                type = i;
            }
        }

        switch(type) {
            case RGBA32:
            return "gsDPLoadTextureBlock " + getFileNameNoExtension() + ", G_IM_FMT_RGBA, G_IM_SIZ_32b, " + std::to_string(get_dimension(AXIS_X, path)) + ", " + std::to_string(get_dimension(AXIS_Y, path)) + ", 0, G_TX_WRAP | G_TX_NOMIRROR, G_TX_WRAP | G_TX_NOMIRROR, " + std::to_string((u16)log2(get_dimension(AXIS_X, path))) + ", " + std::to_string((s16)log2(get_dimension(AXIS_Y, path))) + ", G_TX_NOLOD, G_TX_NOLOD\n";
            break;

            case CI4:
            break;

            case CI8:
            break;

            case RGBA16:
            default:
            printf("[dbg] rgba16\n");
        }
        return "lol";
    }

    /** Returns combiner settings. */
    std::string GetFuckingFrauber() /** TODO: Get this to support fog, semitransparency, and both if possible. */
    {
        if (textured) {
            return "gsDPSetCombineMode G_CCMUX_TEXEL0, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE, G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_COMBINED, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_COMBINED\n";
        }

        else {
            return "gsDPSetCombineMode G_CCMUX_PRIMITIVE, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE, G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_COMBINED, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_COMBINED\n";
        }
    }

    public:
    bool useless = false, textured = false;
    void setName(const std::string &n) { name = n; }
    void setPath(const std::string &p) { path = p; }
    std::string getPath() { return path; }


    std::string getFileNameNoExtension()
    {
        std::string toReturn = get_filename(path);
        return toReturn.substr(0, toReturn.length() - 4); /* <- Hack */
    }

    /** Configures the Material object. (Unused for now) */
    void setMaterial(std::string n, std::string p) {}

    /** Returns the incbins. */
    std::string d;
    /** Returns the exact F3D settings that represent this material. */
    std::string getMaterial()
    {
        std::string ret;
        if (textured) {
            ret = GetFuckingFrauber() + GetTextureLoad();
        }

        else { /* no texture found when setting up */
            ret = GetFuckingFrauber() + "gsDPSetPrimColor 128, 128, 128\n";
        }
        return ret;
    }
};
