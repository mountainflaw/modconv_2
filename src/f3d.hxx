enum BufferModes { RESET, OPTIMIZE, BUFFER };

extern u32 vert;
extern u32 vert2;
extern u16 vBuffers;
extern u16 vBuffer;
extern u16 bone;
extern u8  layers;

struct CullRegion {
    s16 x0;
    s16 x1;
    s16 z0;
    s16 z1;
    s16 y0;
    s16 y1;
};

extern bool gCullDlist;

void setup_vtx(aiNode *node, const aiScene* scene, s16 scale, VertexBuffer* vBuf, const std::string &file, Material* mat);
void cycle_vbuffers(VertexBuffer *vBuf, u8 mode, u8 microcode);
void inspect_vtx(aiNode* node, const aiScene* scene);
void configure_materials(const std::string &file, const std::string &fileOut, Material* mat, const aiScene* scene);
