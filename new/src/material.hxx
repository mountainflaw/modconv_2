class Material
{
    private:
    std::string path, name;

    std::string GetTextureLoad()
    {
        return "lol";
    }

    std::string GetFuckingFrauber()
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


    /** Configures the Material object. (Unused for now) */
    void setMaterial(std::string n, std::string p) {}

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
