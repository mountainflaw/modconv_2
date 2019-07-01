
/*
 * Display list class
 * Used to build the up to 8 different types of displaylists.
 */

#define MAT_NOT_LAYER -2

class DisplayList
{
    private:
    std::string dlTypes[8] = {"_layer_0", "_layer_1", "_layer_2", "_layer_3", "_layer_4", "_layer_5", "_layer_6", "_layer_7"};
    u8 layer = 1;

    bool writeTri(s16 tri[], u8 size)
    {
        for (u8 i = 0; i < size; i++) {
            if (tri[i] == -1) {
                std::cout << "tri ind " << tri[i] << std::endl;
                return false;
            }
        }
        return true;
    }

    public:
    void setLayer(u8 l) { layer = l; }

    void writeDisplayList(const std::string &fileOut, VertexBuffer *vBuf, u16 vBuffers, Material* mat)
    {
        bool oldGeo[5] = {false};
        std::fstream gfxOut;
        s16 currMat = -1; /* force update at start*/

        gfxOut.open(fileOut + "/model.s", std::ofstream::out | std::ofstream::app);
        gfxOut << std::endl << "glabel " << fileOut + dlTypes[layer] << std::endl;
        std::cout << "opened " << fileOut + "/model.s" << std::endl;
        for (u16 i = 0; i < vBuffers; i++) {
            vBuf[i].vtxCount = 0;
            vBuf[i].vtxCount2 = 0;
            if (vBuf[i].hasLayer(layer)) { /* don't load what we don't need */
                std::cout << "vtx " << i << " contains layer " << (u16)layer << std::endl;
                if (vBuf[i].getLayeredVtxMat(layer) != currMat && vBuf[i].getLayeredVtxMat(layer) != MAT_NOT_LAYER) { /* load before vtx load if possible */
                    currMat = vBuf[i].getLayeredVtxMat(layer);
                    gfxOut << "/* " << mat[currMat].getName() << " */" << std::endl
                                    << mat[currMat].getMaterial(oldGeo);
                }
                gfxOut << "gsSPVertex " << get_filename(fileOut) << "_vertex_" << i << " " << std::to_string(vBuf[i].loadSize) << ", 0" << std::endl;

                while (!vBuf[i].isBufferComplete()) {
                    vBuf[i].vtxCount2 = vBuf[i].vtxCount;
                    if (vBuf[i].getVtxMat() != currMat && vBuf[i].getLayeredVtxMat(layer) != MAT_NOT_LAYER) {
                        currMat = vBuf[i].getLayeredVtxMat(layer);
                        gfxOut << "/* " << mat[currMat].getName() << " */" << std::endl
                                        << mat[currMat].getMaterial(oldGeo);
                    }

                    if (vBuf[i].canLayeredTri2(layer)) {
                   // if (false) {
                        s16 triTwo[6] = { vBuf[i].getLayeredVtxIndex(layer), vBuf[i].getLayeredVtxIndex(layer), vBuf[i].getLayeredVtxIndex(layer),
                                          vBuf[i].getLayeredVtxIndex(layer), vBuf[i].getLayeredVtxIndex(layer), vBuf[i].getLayeredVtxIndex(layer) };
                        if (writeTri(triTwo, 6)) {
                            gfxOut << "gsSP2Triangles "
                                   << triTwo[0] << ", "
                                   << triTwo[1] << ", "
                                   << triTwo[2] << ", 0x00, "
                                   << triTwo[3] << ", "
                                   << triTwo[4] << ", "
                                   << triTwo[5] << ", 0x00"
                                   << std::endl;
                        }
                    } else {
                        s16 triOne[3] = { vBuf[i].getLayeredVtxIndex(layer), vBuf[i].getLayeredVtxIndex(layer), vBuf[i].getLayeredVtxIndex(layer) };
                        if (writeTri(triOne, 3)) {
                            gfxOut << "gsSP1Triangle "
                                   << triOne[0] << ", "
                                   << triOne[1] << ", "
                                   << triOne[2] << ", 0x00" << std::endl;
                        }
                    }
                }
            } else {
                std::cout << "buffer " << (u16)i << " does not have layer " << layer << std::endl;
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
