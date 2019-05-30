/*
*   Copyright (c) 2019, Red                                                             *
*   All rights reserved.                                                                *
*                                                                                       *
*   Redistribution and use in source and binary forms, with or without                  *
*   modification, are permitted provided that the following conditions are met:         *
*                                                                                       *
*       * Redistributions of source code must retain the above copyright                *
*         notice, this list of conditions and the following disclaimer.                 *
*       * Redistributions in binary form must reproduce the above copyright             *
*         notice, this list of conditions and the following disclaimer in the           *
*         documentation and/or other materials provided with the distribution.          *
*       * Neither the name of the modconv 2 developers nor the                          *
*         names of its contributors may be used to endorse or promote products          *
*         derived from this software without specific prior written permission.         *
*                                                                                       *
*   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND     *
*   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED       *
*   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE              *
*   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY                *
*   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES          *
*   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;        *
*   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND         *
*   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
*   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS       *
*   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                        *
*/

/*
 * New F3D builder.
 * Differences from the last:
 * 1.) It no longer assumes vertices when loading the vertex buffers, meaning that stray
 * vertices no longer can cause problems with importing models.
 * 2.) The code is a lot cleaner.
 * 3.) The display list builder is no longer its own file since it is just a loop.
 * 4.) The material class has been simplified.
 */

enum RgbaColors { C_RED, C_GRN, C_BLU, C_APH };

#include "modconv.hxx"
#include "f3d.hxx"
#include "buffer.hxx"
#include "material.hxx"

u32 vert     = 0,
    vert2    = 0;
u16 meshId   = 0,
    vBuffers = 0,
    vBuffer  = 0;

static void count_vtx(aiNode* node, const aiScene* scene)
{
    for (u16 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        vert += mesh->mNumFaces * 3;
        meshId++;
    }

    for (u16 i = 0; i < node->mNumChildren; i++) {
        count_vtx(node->mChildren[i], scene);
    }
}

/** Add vertices to vertex buffers. */
static void setup_vtx(aiNode *node, const aiScene* scene, s16 scale,
        VertexBuffer* vBuf, const std::string &file, bool yUp)
{
    for (u16 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        /* we go by faces instead of verts so we don't accidentally add what we don't need */
        for (u32 j = 0; j < mesh->mNumFaces; j++) {
            printf("[dbg] face %d\n", j);
            for (u8 k = 0; k <= 2; k++) {
                u32 currVtx = mesh->mFaces[j].mIndices[k];

                if (vBuf[vBuffer].isBufferComplete()) {
                    vBuf[vBuffer].vtxCount = 0;
                    vBuffer++;

                    if (vBuffer == vBuffers - 1) { /* set the max amount for the final vbuffer */
                        vBuf[vBuffer].bufferSize = vert - vert2;
                    }
                }

                s16 pos[3];

                if (yUp) { /* y axis up */
                    pos[AXIS_X] = (s16)mesh->mVertices[currVtx].x * scale;
                    pos[AXIS_Y] = (s16)mesh->mVertices[currVtx].y * scale;
                    pos[AXIS_Z] = (s16)mesh->mVertices[currVtx].z * scale;
                }

                else { /* default setting (z axis up) */
                    pos[AXIS_X] = (s16)mesh->mVertices[currVtx].x * scale;
                    pos[AXIS_Y] = (s16)mesh->mVertices[currVtx].z * scale;
                    pos[AXIS_Z] = (s16)mesh->mVertices[currVtx].y * scale;
                }

                s16 uv[2] = {0x00};

                /* We have to look at material data so we can multiply the UV data. */
                if (scene->HasMaterials()) { /* ditto */
                    aiString aiPath;
                    scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &aiPath);
                    std::string path = aiPath.data;

                    if (file_exists(path)) { /* absolute */
                        std::cout << "[dbg] file exists! - " << path << std::endl;
                        uv[AXIS_X] = mesh->mTextureCoords[0][currVtx].x * 32 * get_dimension(AXIS_X, path);
                        uv[AXIS_Y] = mesh->mTextureCoords[0][currVtx].y * 32 * get_dimension(AXIS_Y, path);
                        printf("[dbg] uv is %d %d\n", uv[AXIS_X], uv[AXIS_Y]);
                    }

                    else if (file_exists(get_path(file) + path) && !(is_directory(get_path(file) + path))) { /* relative */
                        std::cout << "[dbg] file exists (relative)! - " << file + path << std::endl;
                        uv[AXIS_X] = mesh->mTextureCoords[0][currVtx].x * 32 * get_dimension(AXIS_X, get_path(file) + path);
                        uv[AXIS_Y] = mesh->mTextureCoords[0][currVtx].y * 32 * get_dimension(AXIS_Y, get_path(file) + path);
                        printf("[dbg] uv is %d %d\n", uv[AXIS_X], uv[AXIS_Y]);
                    }

                    else { /* no texture found */
                        std::cout << "[dbg] no texture found - abs " << path << " - relative - " << file + path << std::endl;
                    }
                }

                u8 rgba[4] = {0xff, 0xff, 0xff, 0xff};

                if (mesh->HasVertexColors(0)) { /* Get around potential exception. */
                    rgba[C_RED] = mesh->mColors[0][currVtx].r * 0xff;
                    rgba[C_GRN] = mesh->mColors[0][currVtx].g * 0xff;
                    rgba[C_BLU] = mesh->mColors[0][currVtx].b * 0xff;
                    rgba[C_APH] = mesh->mColors[0][currVtx].a * 0xff;
                }

                printf("[dbg] vtx %d %d %d\n", pos[AXIS_X], pos[AXIS_Y], pos[AXIS_Z]);
                vBuf[vBuffer].addVtx(pos[AXIS_X], pos[AXIS_Y], pos[AXIS_Z],
                        uv[AXIS_X], uv[AXIS_Y],
                        rgba[C_RED], rgba[C_GRN], rgba[C_BLU], rgba[C_APH], i);
                vert2++;
            }
        }
    }
}

enum BufferModes {RESET, OPTIMIZE, BUFFER};

/** Function for common vbuffer operations (reset counter, run the optimizer, etc). */
static inline void cycle_vbuffers(VertexBuffer *vBuf, u8 mode, u8 microcode)
{
    switch (mode) {
        case BUFFER:
        for (u16 i = 0; i < vBuffers; i++) {
            vBuf[i].bufferSize = microcode;
        }
        break;

        case OPTIMIZE:
        for (u16 i = 0; i < vBuffers; i++) {
            vBuf[i].vtxCount = 0;
        }

        case RESET:
        for (u16 i = 0; i < vBuffers; i++) {
            vBuf[i].vtxCount = 0;
        }
        break;
    }
}

static void write_vtx(const std::string fileOut, const std::string &path, VertexBuffer *vBuf)
{
    std::fstream vtxOut;
    vtxOut.open(fileOut + "/model.s", std::ofstream::out | std::ofstream::app);
    for (u16 i = 0; i < vBuffers; i++) {
        vtxOut << std::endl << fileOut << "_vertex_" << i << ":" << std::endl;
        for (u16 j = 0; j < vBuf[i].bufferSize; j++) {
            Vertex vtx = vBuf[i].getVtx();
            vtxOut << "vertex " << vtx.pos[AXIS_X] << ", "
                << vtx.pos[AXIS_Y] << ", "
                << vtx.pos[AXIS_Z] << ", "
                << vtx.st[AXIS_X]  << ", "
                << vtx.st[AXIS_Y]  << ", "
                << (u16)vtx.col[C_RED]  << ", "
                << (u16)vtx.col[C_GRN]  << ", "
                << (u16)vtx.col[C_BLU]  << ", "
                << (u16)vtx.col[C_APH]  << std::endl;
        }
    }
}

static void configure_materials(const std::string &file, Material* mat, aiNode* node, const aiScene* scene)
{
    for (u16 i = 0; i < node->mNumMeshes; i++) {
        aiString aiPath, aiName;
        scene->mMaterials[i]->Get(AI_MATKEY_NAME, aiName);
        scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &aiPath);

        if (file_exists(aiPath.data)) { /* absolute */
            std::cout << "[dbg] absolute: " << get_path(file) << std::endl;
            mat[meshId].setPath(aiPath.data);
            mat[meshId].textured = true;
        }

        else if (file_exists(get_path(file) + aiPath.data) && !(is_directory(get_path(file) + aiPath.data))) { /* relative */
            std::cout << "[dbg] relative: " << get_path(file) + aiPath.data << std::endl;
            mat[meshId].setPath(get_path(file) + aiPath.data);
            mat[meshId].textured = true;
        }

        meshId++;
    }

    for (u16 i = 0; i < node->mNumChildren; i++) {
        configure_materials(file, mat, node->mChildren[i], scene);
    }
}

static void write_textures(const std::string &fileOut, Material *mat, bool level)
{
    std::fstream texOut;
    if (level) {
        reset_file(fileOut + "/texture.s");
        texOut.open(fileOut + "/texture.s", std::ofstream::out | std::ofstream::app);
    }

    else { /* generating an actor */
        texOut.open(fileOut + "/model.s", std::ofstream::out | std::ofstream::app);
    }

    /* Phase 1 - Find redundant textures */
    for (u16 i = 0; i < meshId; i++) {
        for (u16 j = 0; j < meshId; j++) {
            if (mat[i].getPath().compare(mat[j].getPath()) == 0 && j > i) {
                mat[j].useless = true;
            }
        }
    }

    /* Phase 2: Write and copy textures */
    for (u16 i = 0; i < meshId; i++) {
        if (!mat[i].useless) {
            texOut << std::endl;
            texOut << mat[i].getFileNameNoExtension() << ":" << std::endl;
            if (level) {
                texOut << ".incbin " << R"(")" << "levels/" << fileOut << "/" << mat[i].getFileNameNoExtension() << R"(")" << std::endl;
            }

            else { /* generating an actor */
                texOut << ".incbin " << R"(")" << "actors/" << fileOut << "/" << mat[i].getFileNameNoExtension() << R"(")" << std::endl;
            }

            if (file_exists(mat[i].getPath())) {
                remove_file(fileOut + "/" + get_filename(mat[i].getPath()));
            }

            copy_file(mat[i].getPath(), fileOut);
        }
    }
}

/** Write display list commands to file. */
/*
 * PROCESS:
 * 1.) Setup (Add glabel and disable G_LIGHTING)
 * 2.) Check if materials are different by looking at the mesh ids a vertex corresponds to.
 * 3.) Check if TRI2 is possible within either the vertex buffer range AND if it doesnt go into
 * a vertex with a different material.
 * 4.) End displaylist after all of that crap is done.
 */
static void write_display_list(const std::string &fileOut, VertexBuffer* vBuf, Material* mat)
{
    std::fstream gfxOut;
    gfxOut.open(fileOut + "/model.s", std::ofstream::out | std::ofstream::app);

    u16 currMat = -1;
    gfxOut << std::endl << "glabel " << fileOut << "_dl" << std::endl
        << "gsSPClearGeometryMode G_LIGHTING" << std::endl;
    for (u16 i = 0; i < vBuffers; i++) {
        gfxOut << "gsSPVertex " <<  fileOut << "_vertex_" << i
            << " " << std::to_string(vBuf[i].bufferSize) << ", 0" << std::endl;
        while (!vBuf[i].isBufferComplete()) {
            if (vBuf[i].getVtxMat() != currMat) {
                gfxOut << mat[++currMat].getMaterial();
            }
            gfxOut << "gsSP1Triangle " << vBuf[i].getVtxIndex() << " "
                << vBuf[i].getVtxIndex() << " "
                << vBuf[i].getVtxIndex() << std::endl;
        }
    }
    gfxOut << "gsSPTexture -1, -1, 0, 0, 0\ngsSPEndDisplayList" << std::endl;
}

/** Main function for the F3D build process. */
void f3d_main(const std::string &file, const std::string &fileOut, s16 scale, u8 microcode, bool level, bool yUp)
{
    Assimp::Importer importer;

    /* We don't use ASSIMP's built in tristripping because of the vertex buffer. */
    const aiScene* scene = importer.ReadFile(file, aiProcess_ValidateDataStructure | aiProcess_Triangulate);

    reset_file(fileOut + "/model.s");
    count_vtx(scene->mRootNode, scene);

    printf("[dbg] there are %d verts\n", vert);
    vBuffers = vert / microcode;

    if (vert % microcode > 0) { /* is there a trailing vbuffer? */
        vBuffers++;
    }

    VertexBuffer vBuf[vBuffers];
    cycle_vbuffers(vBuf, BUFFER, microcode);

    for (u16 i = 0; i < scene->mRootNode->mNumChildren; i++) {
        setup_vtx(scene->mRootNode->mChildren[i], scene, scale, vBuf, file, yUp);
    }

    std::cout << "[dbg] There are " << vBuffers << " vbuffers." << std::endl;

    /* Materials */
    Material mat[meshId];
    meshId = 0;
    configure_materials(file, mat, scene->mRootNode, scene);
    write_textures(fileOut, mat, level);

    cycle_vbuffers(vBuf, OPTIMIZE, 0);
    write_vtx(fileOut, "", vBuf);
    cycle_vbuffers(vBuf, RESET, 0);
    write_display_list(fileOut, vBuf, mat);
}
