
/*
 * Display list class
 * Used to build the up to 8 different types of displaylists.
 */

class DisplayList
{
    private:
    std::string dlTypes[8] = {"_layer_0", "_layer_1", "_layer_2", "_layer_3", "_layer_4", "_layer_5", "_layer_6", "_layer_7"}, fileOut = "";
    u8 layer = 1;


    public:
    void setupDisplayList(const std::string &f, u8 l)
    {
        fileOut = f;
        layer = l;
    }

    void writeDisplayList(VertexBuffer *vBuf, u16 vBuffers, Material* mat)
    {
        bool oldGeo[5] = {false};
        std::fstream gfxOut;
        s16 currMat = -1; /* force update at start*/

        gfxOut.open(fileOut + "/model.s", std::ofstream::out | std::ofstream::app);
        for (u16 i = 0; i < vBuffers; i++) {
            if (vBuf[i].hasLayer(layer)) { /* don't load what we don't need */
                if (vBuf[i].getVtxMat() != currMat) { /* load before vtx load if possible */
                    gfxOut << "/* " << mat[currMat].getName() << " */" << std::endl
                                    << mat[currMat].getMaterial(oldGeo);
                }

                while (!vBuf[i].isBufferComplete()) {
                    if (vBuf[i].getVtxMat() != currMat) {
                        gfxOut << "/* " << mat[currMat].getName() << " */" << std::endl
                                        << mat[currMat].getMaterial(oldGeo);
                    }

                    if (vBuf[i].canTri2()) {
                        u16 triTwo[6] = { vBuf[i].getVtxIndex(), vBuf[i].getVtxIndex(), vBuf[i].getVtxIndex(),
                                          vBuf[i].getVtxIndex(), vBuf[i].getVtxIndex(), vBuf[i].getVtxIndex() };

                        gfxOut << "gsSP2Triangles "
                               << triTwo[0] << ", "
                               << triTwo[1] << ", "
                               << triTwo[2] << ", 0x00, "
                               << triTwo[3] << ", "
                               << triTwo[4] << ", "
                               << triTwo[5] << ", 0x00"
                               << std::endl;
                    } else {
                        u16 triOne[3] = { vBuf[i].getVtxIndex(), vBuf[i].getVtxIndex(), vBuf[i].getVtxIndex() };

                        gfxOut << "gsSP1Triangle "
                               << triOne[0] << ", "
                               << triOne[1] << ", "
                               << triOne[2] << ", 0x00" << std::endl;
                    }
                }
            }
        }
        gfxOut << "gsSPTexture -1, -1, 0, 0, 0\n"
               << "gsDPPipeSync\n"
               << "gsDPSetCombineMode1Cycle G_CCMUX_0, G_CCMUX_0, G_CCMUX_0, G_CCMUX_SHADE, G_ACMUX_0, G_ACMUX_0, G_ACMUX_0, G_ACMUX_SHADE\n"
               << "gsSPSetGeometryMode G_LIGHTING\n"
               << "gsDPSetTextureLUT G_TT_NONE\n"
               << "gsSPEndDisplayList" << std::endl;
    }
};
