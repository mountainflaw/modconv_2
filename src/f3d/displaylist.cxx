#include "../common.hxx"
#include "vertex.hxx"

class Vertex;
class Material;

void build_displaylist(const std::string fileOut, Vertex *vtx, Material *mat, s32 verts, s8 f3d)
{
    s16 bVert = -1, gVert = 0, matId = -1;
    std::ofstream displayListOut;
    displayListOut.open(fileOut + "/model.s", std::ofstream::out | std::ofstream::app);

    displayListOut << std::endl << "glabel " << fileOut << "_dl" << std::endl;
    for (int i = 0; i < verts;)
    {
        /* Materials */

        if (!(vtx[i].compareMaterials(matId)))
        {
            matId++;
            displayListOut << mat[matId].getMaterial();
        }

        if (!(gVert % f3d))
        {
            gVert = 0;
            bVert++;

            if (verts - i + 1 < f3d)
                displayListOut << "gsSPVertex " << fileOut << "_vertex_" << bVert << " " << (verts - i) << ", 0" << std::endl;

            else
                displayListOut << "gsSPVertex " << fileOut << "_vertex_" << bVert << std::to_string(f3d) << ", 0" << std::endl;
        }

        displayListOut << "gsSP1Triangle " << gVert << ", " << gVert + 1 << ", " << gVert + 2 << std::endl;
        gVert += 3;
        i += 3;
    }
    displayListOut << "gsSPEndDisplayList" << std::endl;
}
