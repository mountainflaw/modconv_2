#include "../common.hxx"
#include "displaylist.hxx"

s32 vert   = 0, //! Used by vertex builder
    verts  = 0; //! Total amount of vertices
s16 meshId = 0; //! Recorded into each vert

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
    aiString path, name; /** Raw path and name strings */
    bool textured = false; /** Is the material textured? */

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
                int textureWrap[2] = {0};
                textured = true;
                std::cout << "DBG - PATH EXISTS!" << std::endl;
                std::cout << pathString << std::endl;
                mat[meshId].setMaterial(get_path(file) + pathString, nameString, 0, 0, 0, textureWrap[AXIS_X], textureWrap[AXIS_Y]);
            }

            else if (file_exists(get_path(file) + pathString) && !(is_directory(get_path(file) + pathString)))
            {
                int textureWrap[2] = {0};
                textured = true;
                std::cout << "DBG - PATH EXISTS (RELATIVE)"
                          << get_path(file) + pathString << std::endl;

                //if (AI_SUCCESS != scene->mMaterials[j]->Get(AI_MATKEY_MAPPINGMODE_U_DIFFUSE(5), textureWrap[AXIS_X]) ||
                //                  scene->mMaterials[j]->Get(AI_MATKEY_MAPPINGMODE_V(j, aiTextureType_DIFFUSE), textureWrap[AXIS_Y]))
                //     mat[meshId].setMaterial(get_path(file) + pathString, nameString, 0, 0, 0, 0, 0);
                //else
                //{
                //    std::cout << "DBG - WARNING: FALLING BACK ON TEXWRAP MODE!" << std::endl;
                    mat[meshId].setMaterial(get_path(file) + pathString, nameString, 0, 0, 0, textureWrap[AXIS_X], textureWrap[AXIS_Y]);
                //}
                //std::cout << "DBG - TEXWRAP XY: " << std::to_string(textureWrap[AXIS_X]) << " " << std::to_string(textureWrap[AXIS_Y]) << std::endl;
            }

            else
            {
                aiColor3D pColor;
                std::cout << "DBG - TEXTURE DOES NOT EXIST! USING BASE COLORS!" <<
                "\nRelative: " << get_path(file) + pathString << "\nAbsolute: " << pathString << std::endl;

                if(AI_SUCCESS != scene->mMaterials[j]->Get(AI_MATKEY_COLOR_DIFFUSE, pColor))
                {
                    std::cout << "DBG - oh god oh fuck time to whip out the default material" << std::endl;
                    mat[meshId].setMaterial("CONV_UNUSED", nameString, 238, 238, 238, 0, 0); /* Set it to a nice #EEEEEEE if it gets fucked somehow */
                }

                else
                {
                    std::cout << "DBG - Safely using primcolors" << std::endl;
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

            if (mesh->HasTextureCoords(0) && textured)
            {
                vtx[vert].setTextureCoords(AXIS_X, mesh->mTextureCoords[0][i].x * 32 * mat[meshId].getDimension(AXIS_X));
                vtx[vert].setTextureCoords(AXIS_Y, mesh->mTextureCoords[0][i].y * 32 * mat[meshId].getDimension(AXIS_Y));
            }

            else
            {
                vtx[vert].setTextureCoords(AXIS_X, 0);
                vtx[vert].setTextureCoords(AXIS_Y, 0);
            }

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

            vtx[vert].setMaterial(meshId);
            vert++;
        }
        meshId++;
    }
}

void write_vtx(Vertex *vtx, const std::string &fileOut, s8 output)
{
    s16 vtxGroup = 0;
    std::ofstream vertexOut;

    vertexOut.open(fileOut + "/model.s", std::iostream::out | std::iostream::app);
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

/** Writes materials to file. */
void write_materials(Material *mat, const std::string &fileOut)
{
    /* PHASE 1: Setup string array */ /* TODO: make this entire function not retarded. */
    std::string matOutputs[meshId] = {"CONV_UNUSED"};
    for (int i = 0; i < meshId; i++)
    {
        std::string matName = mat[i].getFileNameNoExtension();
        if (!(matName.find("CONV_UN") != std::string::npos) && mat[i].isTextured())
        {
            matOutputs[i] = matName + ":\n" + ".incbin " + R"(")" + matName + R"(")" + "\n";
        }
    }

    /* PHASE 2: Optimize the material string array so we don't incbin twice */
    for (int i = 0; i < meshId; i++)
    {
        for (int j = 0; j < meshId; j++)
        {
            if (matOutputs[i].compare(matOutputs[j]) == 0 && i > j)
                matOutputs[i] = "CONV_UN";
        }
    }

    /* PHASE 3: Actually write the materials */
    std::fstream materialOut;
    materialOut.open(fileOut + "/model.s", std::iostream::out | std::iostream::app);
    for (int i = 0; i < meshId; i++)
    {
        if (!(matOutputs[i].find("CONV_UN") != std::string::npos) && (mat[i].isTextured()))
        {
            std::cout << "DBG - a" << std::endl;
            materialOut << matOutputs[i];
        }
        /*else
           materialOut << matOutputs[i];*/
    }
}

/** Main function for the vertex phase. */
void vertex_phase(const std::string &file, const std::string &fileOut, s16 scale, s8 f3d)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file, aiProcess_ValidateDataStructure);

    /* Some file operations */
    reset_directory(fileOut);
    reset_file(fileOut + "/model.s");

    /* Get amount of verts and create vertex objects */
    get_num_of_verts(scene->mRootNode, scene);
    Vertex vtx[verts];
    Material mat[meshId];
    meshId = 0;
    std::cout << "DBG - Number of verts: " << std::to_string(verts) << std::endl;

    for (s16 i = 0; i < scene->mRootNode->mNumChildren; i++)
        setup_vtx(scene->mRootNode->mChildren[i], scene, scale, vtx, mat, fileOut, file);

    write_materials(mat, fileOut);

    /* One day: optimize verts */
    /* Output vertices */
    write_vtx(vtx, fileOut, f3d);

    /* Build display list */
    build_displaylist(fileOut, vtx, mat, verts, f3d);
}
