#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>

#include <math.h>

//#include "vertex.hxx"
#include "../common.hxx"

class vertex
{
    private:
    s16  pos[3], st[2], mat;
    s32  flag[2];
    s8   col[4];

    public:
    bool compareMaterials(s16 material)
    {
        if (material == mat)
            return true;
        else return false;
    }

    s16 getPos(s8 axis)            { return pos[axis]; }
    s16 getTextureCoords(s8 axis)  { return st[axis]; }
    s16 getMaterial()              { return mat; }
    s32 getFlag(s8 axis)           { return flag[axis]; }
    s8  getColor(s8 channel)       { return col[channel]; }

    void setPos(s8 axis, s16 val)            { pos[axis] = val; }
    void setTextureCoords(s8 axis, s16 val)  { st[axis] = val; }
    void setMaterial(s16 val)                { mat = val; }
    void setFlag(s8 axis, s16 val)           { flag[axis] = val; }
    void setColor(s8 channel, s8 val)        { col[channel] = val; }
};

/* Now behold, THE MOST complex part of the entire converter */

class material
{
    private:
    s16 type, dimension[4], textype;
    s8 primcolors[3];
    std::string path, name, texmode[2];
    bool ourgeo[8];

    //std::string GetPalette(int mode) /* Creates palettes for CI and I(A) */
    /*{
        switch (mode)
        {
            case
        }
    }*/

    void SetTextureType(const std::string &filename)
    {

        /* RGBA */

        if (filename.find("rgba32") != std::string::npos)
        {
            textype = RGBA32;
            std::cout << "DBG - Texture is of type RGBA32!" << std::endl;
        }

        else if (filename.find("rgba16") != std::string::npos)
        {
            textype = RGBA16;
            std::cout << "DBG - Texture is of type RGBA16!" << std::endl;
        }

        /* IA16 */

        else if (filename.find("ia16") != std::string::npos)
        {
            textype = IA16;
            std::cout << "DBG - Texture is of type IA16!" << std::endl;
        }

        /* CI8 */

        else if (filename.find("ci8") != std::string::npos)
        {
            textype = CI8;
            std::cout << "DBG - Texture is of type CI8!" << std::endl;
        }

       /* IA8 */

       else if (filename.find("ia8") != std::string::npos)
       {
            textype = IA8;
            std::cout << "DBG - Texture is of type IA8!" << std::endl;
       }

       /* I8 */

       else if (filename.find("i8") != std::string::npos)
       {
            textype = I8;
            std::cout << "DBG - Texture is of type I8!" << std::endl;
       }

       /* CI4 */

       else if (filename.find("ci4") != std::string::npos)
       {
            textype = CI4;
            std::cout << "DBG - Texture is of type CI4!" << std::endl;
       }

       /* IA4 */

       else if (filename.find("ia4") != std::string::npos)
       {
            textype = IA4;
            std::cout << "DBG - Texture is of type IA4!" << std::endl;
       }

       /* I4 */

       else if (filename.find("i4") != std::string::npos)
       {
            textype = I4;
            std::cout << "DBG - Texture is of type I4!" << std::endl;
       }
    }

    std::string GetTextureLoad()
    {
        switch (textype)
        {
            case RGBA32:
                return "gsDPLoadTextureBlock " + name + ", G_IM_FMT_RGBA, G_IM_SIZ_32b, " + "0, " + texmode[0] + ", "  + texmode[1] + ", " + std::to_string((int)log2(dimension[0])) + ", " + std::to_string((int)log2(dimension[1])) + ", G_TX_NOLOD, G_TX_NOLOD\n";
                break;

            case RGBA16:
                return "gsDPLoadTextureBlock " + name + ", G_IM_FMT_RGBA, G_IM_SIZ_16b, " + "0, " + texmode[0] + ", "  + texmode[1] + ", " + std::to_string((int)log2(dimension[0])) + ", " + std::to_string((int)log2(dimension[1])) + ", G_TX_NOLOD, G_TX_NOLOD\n";
                break;

            default:
                exit(1);
                //error_message("Texture type not implemented.");
        }
    }

    std::string GetSetCombine()
    {
        switch (type)
	{
            case SOLID_COLOR:
                return "gsDPSetCombineMode1Cycle G_CCMUX_PRIMITIVE0, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE\n";
		break;
	    default:
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
                toReturn += "gsDPSetPrimColor 0, 0, " + std::to_string(primcolors[0]) + ", " + std::to_string(primcolors[1]) + ", " + std::to_string(primcolors[2]) + "\n";
                return toReturn;
                break;

            case TEXTURED:
                 toReturn = "gsDPPipeSync\ngsDPSetCombineMode1Cycle G_CCMUX_TEXEL0, G_CCMUX_0, G_CCMUX_SHADE, G_CCMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE\n";
                 toReturn += GetTextureLoad();
                 return toReturn;

            case ENV_MAPPED:
                toReturn = "gsDPPipeSync\ngsDPSetEnvColor " + std::to_string(primcolors[0]) + ", " + std::to_string(primcolors[1]) + ", " + std::to_string(primcolors[2]) + ", 255, 255\n";
                toReturn += "gsSPSetGeometryMode G_LIGHTING | G_TEXTURE_GEN\n";
                toReturn += GetSetCombine();
                toReturn += GetTextureLoad();
                return toReturn;
                break;
        }
    }

    std::string getGeometryMode(bool *geo)
    {
        std::string geobuffer = "", cleargeobuffer = "";
        bool newgeo = false, oring = false, clear = false;

        /* Environment mapping */

        if (ourgeo[TEXGEN] && !(geo[TEXGEN])) /* Enable */
        {
            newgeo = true;
            oring = true;
            geobuffer += "G_TEXTURE_GEN";
        }

        if (!(ourgeo[TEXGEN]) && geo[TEXGEN]) /* Disable */
        {
            clear = true;
            oring = true;
            cleargeobuffer += "G_TEXTURE_GEN";
        }

        /* Backface culling */

        if (ourgeo[TEXGEN] && !(geo[TEXGEN])) /* Enable */
        { 
            newgeo = true;
            if (oring)
                geobuffer += " | G_TEXTURE_GEN";
            else
                geobuffer += "G_TEXTURE_GEN";
        }

        if (!(ourgeo[TEXGEN]) && geo[TEXGEN]) /* Disable */
        {
            clear = true;
            if (oring)
                cleargeobuffer += "| G_TEXTURE_GEN";
            else
                cleargeobuffer += "G_TEXTURE_GEN";
        }

        if (newgeo)
            geobuffer      = "gsSPSetGeometryMode " + geobuffer + "\n";
        else geobuffer = "";

        if (clear)
            cleargeobuffer = "gsSPClearGeometryMode " + cleargeobuffer + "\n";
        else cleargeobuffer = "";

        return geobuffer + cleargeobuffer;
    }
};
