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

        switch (f3d) /* todo: make this not retarded. a switch is not needed. */
        {
            case OUTPUT_F3D:
                if (!(gVert % OUTPUT_F3D))
                {
                    gVert = 0;
                    bVert++;

                    if (verts - i + 1 < 15)
                        displayListOut << "gsSPVertex " << fileOut << "_vertex_" << bVert << " " << (verts - i) << ", 0" << std::endl;

                    else
                        displayListOut << "gsSPVertex " << fileOut << "_vertex_" << bVert << " 15, 0" << std::endl;
                }
                break;

            case OUTPUT_F3DEX:
                if (!(gVert % OUTPUT_F3DEX))
                {
                    gVert = 0;
                    bVert++;

                    if (verts - i + 1 < 30)
                        displayListOut << "gsSPVertex " << fileOut << "_vertex_" << bVert << " " << (verts - i) << ", 0" << std::endl;

                    else
                        displayListOut << "gsSPVertex " << fileOut << "_vertex_" << bVert << " 30, 0" << std::endl;
                }
                break;

            case OUTPUT_REJ:
                if (!(gVert % OUTPUT_REJ))
                {
                    gVert = 0;
                    bVert++;

                    if (verts - i + 1 < 63)
                        displayListOut << "gsSPVertex " << fileOut << "_vertex_" << bVert << " " << (verts - i) << ", 0" << std::endl;

                    else
                        displayListOut << "gsSPVertex " << fileOut << "_vertex_" << bVert << " 63, 0" << std::endl;
                }
                break;

            case OUTPUT_REJ2:
                if (!(gVert % OUTPUT_REJ2))
                {
                    gVert = 0;
                    bVert++;

                    if (verts - i + 1 < 15)
                        displayListOut << "gsSPVertex " << fileOut << "_vertex_" << bVert << " " << (verts - i) << ", 0" << std::endl;

                    else
                        displayListOut << "gsSPVertex " << fileOut << "_vertex_" << bVert << " 78, 0" << std::endl;
                }
                break;
        }

            displayListOut << "gsSP1Triangle " << gVert++ << ", " << gVert++ << ", " << gVert++ << std::endl;
            i +=3;
    }
    displayListOut << "gsSPEndDisplayList" << std::endl;
}
