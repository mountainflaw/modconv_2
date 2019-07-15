enum BufferModes { RESET, OPTIMIZE, BUFFER };

extern u32 vert;
extern u32 vert2;
extern u16 vBuffers;
extern u16 vBuffer;
extern u8  layers;

void setup_vtx(aiNode *node, const aiScene* scene, s16 scale, VertexBuffer* vBuf, const std::string &file, Material* mat);
void cycle_vbuffers(VertexBuffer *vBuf, u8 mode, u8 microcode);
void inspect_vtx(aiNode* node, const aiScene* scene);
void configure_materials(const std::string &file, const std::string &fileOut, Material* mat, const aiScene* scene);
