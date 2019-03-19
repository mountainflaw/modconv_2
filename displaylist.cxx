/* Bigass function that builds the display list */

#include "include.hxx"

void buildDisplayList(std::string fileOut, int tVerts, std::string *mats)
{
    std::cout << mats[0] << std::endl;
    int bVert = -1, /* This is set to -1 because it's immediately added to when the DL generation begins. */
    gVert     =  0, /* Amount of verts in the current buffer */
    tCount    =  0; /* Amount of triangles */
    
    std::ofstream displayListOut;
    std::string fileNameOut = fileOut + "/actor.s";
    displayListOut.open(fileNameOut, std::ofstream::out | std::ofstream::app);
    displayListOut << "\nglabel " << fileOut << "_dl" << std::endl;

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
        displayListOut << "gsSP1Triangle " << gVert++ << ", " << gVert++ << ", " << gVert++ << std::endl;
        i +=3;
    }
}

void prepareDisplayList(std::string fileOut, struct vertex *vtx, int f3d, int tVerts, std::string *mats)
{
    std::cout << mats[0] << std::endl;
    infoMessage(INFO_PREPARING_DL);
    buildDisplayList(fileOut, tVerts, mats);
}
