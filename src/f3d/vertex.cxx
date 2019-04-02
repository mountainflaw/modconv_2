#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>

#include <math.h>

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../common.hxx"
#include "../file.hxx"

s32 vert   = 0, //! Used by vertex builder
    verts  = 0; //! Total amount of vertices
s16 meshId = 0; //! Recorded into each vert

class Vertex
{
    private:
    s16  pos[3], st[2], mat;
    s32  flag[2];
    u8   col[4];

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
    s16 getColor(s8 channel)       { return (s16)col[channel]; }

    void setPos(s8 axis, s16 val)            { pos[axis] = val; }
    void setTextureCoords(s8 axis, s16 val)  { st[axis] = val; }
    void setMaterial(s16 val)                { mat = val; }
    void setFlag(s8 axis, s16 val)           { flag[axis] = val; }
    void setColor(s8 channel, u8 val)        { col[channel] = val; }
};

/* Now behold, THE MOST complex part of the entire converter */

class Material
{
    private:
    s16 type, dimension[2], textype;
    u8 primcolors[3];
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
                return "gsDPLoadTextureBlock " + name + ", G_IM_FMT_RGBA, G_IM_SIZ_32b, " + "0, " + texmode[0] + ", "  + texmode[1] + ", " + std::to_string((s8)log2(dimension[0])) + ", " + std::to_string((s8)log2(dimension[1])) + ", G_TX_NOLOD, G_TX_NOLOD\n";
                break;

            case RGBA16:
                return "gsDPLoadTextureBlock " + name + ", G_IM_FMT_RGBA, G_IM_SIZ_16b, " + "0, " + texmode[0] + ", "  + texmode[1] + ", " + std::to_string((s8)log2(dimension[0])) + ", " + std::to_string((s8)log2(dimension[1])) + ", G_TX_NOLOD, G_TX_NOLOD\n";
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
    void setMaterial(const std::string &path, const std::string &name, u8 primr, u8 primg, u8 primb, s8 modein_x, s8 modein_y)
    {
        /* Disable backface culling */
        if (name.find("#backface") != std::string::npos)
            ourgeo[BKFACE] = true;
        else ourgeo[BKFACE] = false;

        /* Environment mapping */
        if (name.find("#envmap") != std::string::npos)
            ourgeo[TEXGEN] = true;
        else ourgeo[TEXGEN] = false;

        if (path.compare("CONV_UNUSED") == 0)
        {
            type = SOLID_COLOR;
            primcolors[C_RED]   = primr;
            primcolors[C_GREEN] = primg;
            primcolors[C_BLUE]  = primb;
        }

        else
        {
            type = TEXTURED;
            SetTextureType(path);
            dimension[AXIS_X] = get_dimension(AXIS_X, path);
            dimension[AXIS_Y] = get_dimension(AXIS_Y, path);
            texmode[AXIS_X] = modein_x;
            texmode[AXIS_Y] = modein_y;
        }
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

void get_num_of_verts(aiNode* node, const aiScene* scene)
{
    for (s16 i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]]; 
        for(s32 i = 0; i < mesh->mNumVertices; i++)
            verts++;
        meshId++;
    }

    for (s16 i = 0; i < node->mNumChildren; i++)
        get_num_of_verts(node->mChildren[i], scene);
}

/** Grabs the vertex data from ASSIMP and places it inside the vertex objects. */
void setup_vtx(aiNode* node, const aiScene* scene, s16 scale, Vertex *vtx, Material *mat, const std::string &output, const std::string &file)
{
    u8 col[3]; /** Color data for primitives */
    aiString path, name;
    bool texturedMaterial = false;

    for (s16 j = 0; j < node->mNumMeshes; j++)
    {
        if (scene->HasMaterials())
        {
            scene->mMaterials[j]->GetTexture(aiTextureType_DIFFUSE, 0, &path);
            scene->mMaterials[j]->Get(AI_MATKEY_NAME, name);

            std::string nameString = name.data,
                        pathString = path.data;

            if (std::filesystem::exists(pathString))
            {
                std::cout << "DBG - PATH EXISTS!" << std::endl;
                std::cout << pathString << std::endl;
            }

            else if (file_exists(get_path(file) + pathString) && !(is_directory(get_path(file) + pathString)))
            {
                int textureWrap[2] = {0};
                std::cout << "DBG - PATH EXISTS (RELATIVE)"
                          << get_path(file) + pathString << std::endl;

                if (AI_SUCCESS != scene->mMaterials[j]->Get(AI_MATKEY_MAPPINGMODE_U_DIFFUSE(5), textureWrap[AXIS_X]) ||
                                  scene->mMaterials[j]->Get(AI_MATKEY_MAPPINGMODE_V(j, aiTextureType_DIFFUSE), textureWrap[AXIS_Y]))
                    mat[meshId].setMaterial(get_path(file) + pathString, nameString, 0, 0, 0, 0, 0);
                else
                {
                    std::cout << "DBG - WARNING: FALLING BACK ON TEXWRAP MODE!" << std::endl;
                    mat[meshId].setMaterial(get_path(file) + pathString, nameString, 0, 0, 0, textureWrap[AXIS_X], textureWrap[AXIS_Y]);
                }
                std::cout << "DBG - TEXWRAP XY: " << std::to_string(textureWrap[AXIS_X]) << " " << std::to_string(textureWrap[AXIS_Y]) << std::endl;
            }

            else
            {
                aiColor3D pColor;
                std::cout << "DBG - TEXTURE DOES NOT EXIST! USING BASE COLORS!" <<
                "\nRelative: " << get_path(file) + pathString << "\nAbsolute: " << pathString << std::endl;

                if(AI_SUCCESS != scene->mMaterials[j]->Get(AI_MATKEY_COLOR_DIFFUSE, pColor))
                {
                    std::cout << "oh god oh fuck time to whip out the default material" << std::endl;
                    mat[meshId].setMaterial("CONV_UNUSED", nameString, 238, 238, 238, 0, 0);
                }

                else
                {
                    std::cout << "DBG - Safely using" << std::endl;
                    mat[meshId].setMaterial("CONV_UNUSED", nameString, pColor.r, pColor.g, pColor.b, 0, 0);
                }
            }
        }

        aiMesh* mesh = scene->mMeshes[node->mMeshes[j]];

        for(s32 i = 0; i < mesh->mNumVertices; i++)
        {
            vtx[vert].setPos(AXIS_X, mesh->mVertices[i].x * scale);
            vtx[vert].setPos(AXIS_Y, mesh->mVertices[i].y * scale);
            vtx[vert].setPos(AXIS_Z, mesh->mVertices[i].z * scale);

            /*if (mesh->HasTextureCoords(0) && mat[meshId].isTexture())
                vtx[vert].setTextureCoords(AXIS_X, mesh->mTextureCoords[0][i].x * 32 * mat[meshId].getDimension(AXIS_X);
                vtx[vert].setTextureCoords(AXIS_Y, mesh->mTextureCoords[0][i].y * 32 * mat[meshId].getDimension(AXIS_Y);*/

            if (mesh->HasVertexColors(0))
            {
                vtx[vert].setColor(C_RED,   (u8)(mesh->mColors[0][i].r * 0xff));
                vtx[vert].setColor(C_GREEN, (u8)(mesh->mColors[0][i].g * 0xff));
                vtx[vert].setColor(C_BLUE,  (u8)(mesh->mColors[0][i].b * 0xff));
                vtx[vert].setColor(C_ALPHA, (u8)(mesh->mColors[0][i].a * 0xff));
            }

            else
            {
                vtx[vert].setColor(C_RED,   0xff);
                vtx[vert].setColor(C_GREEN, 0xff);
                vtx[vert].setColor(C_BLUE,  0xff);
                vtx[vert].setColor(C_ALPHA, 0xff);
            }
            vert++;
        }
        meshId++;
    }
}

void write_vtx(Vertex *vtx, const std::string &fileOut, s8 output)
{
    s16 vtxGroup = 0;
    std::ofstream vertexOut;

    vertexOut.open(fileOut + "/model/model.s", std::iostream::out | std::iostream::app);
    for (s32 i = 0; i < verts; i++)
    {
        switch (output)
        {
            case OUTPUT_F3D:
                if (!(i % OUTPUT_F3D)) /* 16 vtx buffer */
                    vertexOut << "\n" << fileOut << "_vertex_" << vtxGroup++ << ":" << std::endl;
            break;

            case OUTPUT_F3DEX: /* 32 vtx buffer */
                if (!(i % OUTPUT_F3DEX))
                    vertexOut << "\n" << fileOut << "_vertex_" << vtxGroup++ << ":" << std::endl;
            break;

            case OUTPUT_REJ: /* 64 vtx buffer */
                if (!(i % OUTPUT_REJ))
                    vertexOut << "\n" << fileOut << "_vertex_" << vtxGroup++ << ":" << std::endl;
            break;

            case OUTPUT_REJ2: /* 80 vtx buffer */
                if (!(i % OUTPUT_REJ2))
                    vertexOut << "\n" << fileOut << "_vertex_" << vtxGroup++ << ":" << std::endl;
            break;
        }

        if (1) /* Optimizer */
        {
            vertexOut << "vertex " << vtx[i].getPos(AXIS_X) << ", " << vtx[i].getPos(AXIS_Y) << ", " << vtx[i].getPos(AXIS_Z) << ",    \t";
            vertexOut << vtx[i].getTextureCoords(AXIS_X) << ", " << vtx[i].getTextureCoords(AXIS_Y) << ",    \t" << vtx[i].getColor(C_RED) << ", "; 
            vertexOut << vtx[i].getColor(C_GREEN) << ", " << vtx[i].getColor(C_BLUE) << ", " << vtx[i].getColor(C_ALPHA) << "  \t\t# VERTEX #" << i + 1 << std::endl;
        }
    }
}

/**
 * Main function for vertex phase. Calls the vertex generator and vertex builder, sets up materials, etc. It consists of seven stages:
 * 1.) Find the amount of meshes/materials
 * 2.) Create and modify material objects
 * 3.) Add texture includes
 * 4.) Create vertex objects
 * 5.) Get vertices from ASSIMP
 * 6.) Optimize vertices
 * 7.) Print vertices
 */
void vertex_phase(const std::string &file, const std::string &fileOut, s16 scale, s8 f3d)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file, aiProcess_ValidateDataStructure);

    /* Some file operations */
    reset_directory(fileOut);
    reset_directory(fileOut + "/model");
    reset_file(fileOut + "/model/model.s");

    /* Get amount of verts and create vertex objects */
    get_num_of_verts(scene->mRootNode, scene);
    Vertex vtx[verts];
    Material mat[meshId];
    meshId = 0;
    std::cout << "DBG - Number of verts: " << std::to_string(verts) << std::endl;

    for (s16 i = 0; i < scene->mRootNode->mNumChildren; i++)
        setup_vtx(scene->mRootNode->mChildren[i], scene, scale, vtx, mat, fileOut, file);

    /* One day: optimize verts */
    /* Output vertices */
    write_vtx(vtx, fileOut, f3d);
}
