enum BufferModes { RESET, OPTIMIZE, BUFFER };

extern u32 vert;
extern u32 vert2;
extern u16 vBuffers;
extern u16 vBuffer;
extern u8  layers;

void cycle_vbuffers(VertexBuffer *vBuf, u8 mode, u8 microcode);
