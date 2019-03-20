/* Bigass function that builds the display list */

#include "include.hxx"
#include <math.h>

void buildDisplayList(std::string fileOut, int tVerts, std::string *mats, const struct vertex *vtx)
{
    //std::cout << mats[0] << "lol" << std::endl;
    int bVert = -1, /* This is set to -1 because it's immediately added to when the DL generation begins. */
    gVert     =  0, /* Amount of verts in the current buffer */
    tCount    =  0; /* Amount of triangles */
    
    std::ofstream displayListOut;
    std::string fileNameOut = fileOut + "/actor.s";
    displayListOut.open(fileNameOut, std::ofstream::out | std::ofstream::app);
    displayListOut << "\nglabel " << fileOut << "_dl" << std::endl;
    int lastMesh = -1;
    for (int i = 0; i < tVerts;)
    {
        if (!(gVert % 15))
        {
            gVert = 0;
            bVert++;
            //printf("Generating gSPVertex %d\n", bVert);

            if (tVerts - i + 1 < 15)
                displayListOut << "gsSPVertex " << fileOut << "_vertex_" << bVert << " " << (tVerts - i) << ", 0" << std::endl;

            else
                displayListOut << "gsSPVertex " << fileOut << "_vertex_" << bVert << " 15, 0" << std::endl;
        }
        //std::cout << "mesh: " << vtx[0].mesh[0] << std::endl;
        if (vtx[i].mesh[0] > lastMesh)
        {
            lastMesh++;
            displayListOut << "gsDPLoadTextureBlock " << mats[lastMesh] << ", G_IM_FMT_RGBA, " << "G_IM_SIZ_16b, " << vtx[i].mesh[2] << ", " << vtx[i].mesh[3] << ", 0, " << "G_TX_WRAP | G_TX_NOMIRROR, " << "G_TX_WRAP | G_TX_NOMIRROR, " << (int)log2(vtx[i].mesh[2]) << ", " << (int)log2(vtx[i].mesh[3]) << ", " << "G_TX_NOLOD, G_TX_NOLOD" << std::endl;   
        }
        displayListOut << "gsSP1Triangle " << gVert++ << ", " << gVert++ << ", " << gVert++ << std::endl;
        i +=3;
    }
        displayListOut << "gsSPEndDisplayList" << std::endl;
}

void prepareDisplayList(std::string fileOut, const struct vertex *vtx, int f3d, int tVerts, std::string *mats)
{

    std::cout << vtx[0].mesh[0] << std::endl;
    std::cout << mats[0] << std::endl;
    infoMessage(INFO_PREPARING_DL);
    buildDisplayList(fileOut, tVerts, mats, vtx);
}
