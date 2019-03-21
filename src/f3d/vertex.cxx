#include <math.h>

#include "vertex.hxx"
#include "../common.hxx"

class vertex
{
    private:
    s16  pos[3], st[2], mat, flag, col[4];

    public:
    bool compareMaterials(s16 material)
    {
        if (material == mat)
            return true;
        else return false;
    }

    s16 getPos(s16 axis)           { return pos[axis]; }
    s16 getTextureCoords(s16 axis) { return st[axis]; }
    s16 getMaterial()              { return mat; }
    s16 getFlag()                  { return flag; }
    s16 getColor(s16 channel)      { return color[channel]; }

    void setPos(s16 axis, s16 val)           { pos[axis] = val; }
    void setTextureCoords(s16 axis, s16 val) { st[axis] = val; }
    void setMaterial(s16 val)                { mat = val; }
    void setFlag(s16 val)                    { flag = val; }
    void setColor(s16 channel, char val)     { color[channel] = val; }
}

class material
{
    private:
    s16 type, primcolors[3], dimension[4] textype;
    std::string path, name, texmode[2];

    //std::string GetPalette(int mode) /* Creates palettes for CI and I(A) */
    /*{
        switch (mode)
        {
            case 
        }
    }*/

    void SetTextureType(const std::string &filename)
    {
	    
    }

    std::string GetTextureLoad()
    {
        switch (texType)
        {
            case RGBA32:
                return toReturn += "gsDPLoadTextureBlock " + name + ", G_IM_FMT_RGBA, G_IM_SIZ_32b, " + "0, " + texmode[0] + ", "  + texmode[1] + ", " + (int)log2(dimension[0]) + ", " + (int)log2(dimension[1]) + ", G_TX_NOLOD, G_TX_NOLOD\n";
                break;

            case RGBA16:
                return "gsDPLoadTextureBlock " + name + ", G_IM_FMT_RGBA, G_IM_SIZ_16b, " + "0, " + texmode[0] + ", "  + texmode[1] + ", " + (int)log2(dimension[0]) + ", " + (int)log2(dimension[1]) + ", G_TX_NOLOD, G_TX_NOLOD\n";
                break;

                default:
                    error_message("Texture type not implemented.");
        }
    }

    std::string GetSetCombine()
    {
        switch (type)
	{
		case SOLID_COLOR:
                	return "gsDPSetCombineMode1Cycle G_CCMUX_PRIMITIVE0, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE\n";
			break;
		case default:
			return "gsDPSetCombineMode1Cycle G_CCMUX_TEXEL0, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE\n";
			break;
	}
    }

    public:
    std::string getMaterial()
    {
        std::string toReturn;
        
        switch (type)
        {
            case SOLID_COLOR:
		toReturn = "gsDPPipeSync\n" + GetSetCombine();
		toReturn += "gsDPSetPrimColor 0, 0, " + primcolors[0] + ", " + primcolors[1] + ", " + primcolors[2] + "\n";
        	return toReturn;
		break;

            case TEXTURE: /* TODO: Make this support more than just RGBA. */ 
                    toReturn = "gsDPPipeSync\ngsDPSetCombineMode1Cycle G_CCMUX_TEXEL0, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE\n";
                    toReturn += GetTextureLoad();
                    return toReturn;

            case ENV_MAPPED:
                {
                    toReturn = "gsDPPipeSync\ngsDPSetEnvColor 255, 255, 255, 255\ngsSPSetGeometryMode G_LIGHTING | G_TEXTURE_GEN";
                    toReturn += GetSetCombine();
                    toReturn += GetTextureLoad();
                    return toReturn;
                    break;
                }
        }
    }
}
