#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>

#include <math.h>

#include <assimp/Importer.hxx>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../common.hxx"
#include "../file.hxx"

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
    std::string path, texname, name, texmode[2];
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

        else
        {
            textype = RGBA16;
            std::cout << "DBG - You better hope RGBA16 works fucker." << std::endl;
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

    std::string getGeometryMode(bool *geo)
    {
        std::string geobuffer = "", cleargeobuffer = "";
        bool newgeo = false, oring = false, clearoring = false, clear = false;

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
            clearoring = true;
            cleargeobuffer += "G_TEXTURE_GEN";
        }

        /* Backface culling */

        if (ourgeo[TEXGEN] && !(geo[TEXGEN])) /* Enable */
        { 
            clear = true;
            if (clearoring)
                cleargeobuffer += " | G_CULL_BACK";
            else
                cleargeobuffer += "G_CULL_BACK";
        }

        if (!(ourgeo[TEXGEN]) && geo[TEXGEN]) /* Disable */
        {
            newgeo = true;
            if (oring)
                geobuffer += " | G_TEXTURE_GEN";
            else
                geobuffer += "G_TEXTURE_GEN";
        }

        if (newgeo)
            geobuffer  = "gsSPSetGeometryMode " + geobuffer + "\n";
        else geobuffer = "";

        if (clear)
            cleargeobuffer  = "gsSPClearGeometryMode " + cleargeobuffer + "\n";
        else cleargeobuffer = "";

        return geobuffer + cleargeobuffer;
    }

    public:

    /**
     * Public method to set the material type for the dl builder.
     * SYNOPSIS:
     * path is the file path (or name for relative paths) given to us by ASSIMP.
     * name is the material named if assigned one.
     * primX are the primitive colors for red, green, and blue respectively.
     *
     * Note that #backface in particular DISABLES backface culling when enabled,
     * as it is already enabled by default.
     */
    void setMaterial(const std::string &path, const std::string &name, s8 primr, s8 primg, s8 primb)
    {
        /* Disable backface culling */
        if (name.find("#backface") != std::string::npos)
            ourgeo[BKFACE] = true;
        else ourgeo[BKFACE] = false;

        /* Environment mapping */
        if (name.find("#envmap") != std::string::npos)
            ourgeo[TEXGEN] = true;
        else ourgeo[TEXGEN] = false;

        /* Determine if there's a texture */

        //if (path == NULL)
        //    std::cout << "DBG - FLAT SHADED!" << std::endl;
    }

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
        }
    }
};

/** Main function for vertex phase. Calls the vertex generator and vertex builder, sets up materials, etc. It consists of seven stages:
 * 1.) Find the amount of meshes/materials
 * 2.) Create and modify material objects
 * 3.) Add texture includes
 * 4.) Create vertex objects
 * 5.) Get vertices from ASSIMP
 * 6.) Optimize vertices
 * 7.) Print vertices
 */
void vertex_phase(const std::string &file, const std::string &fileOut, int scale, int f3d)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file, aiProcess_ValidateDataStructure);
    /* Reset the directory just in case */
    reset_directory(fileOut);

    /* Get amount of meshes/materials */
    /* Get amount of verts */
    /* Create vert classes */
    /* Get vertices */
    /* One day: optimize verts */
    /* Output vertices */
}
