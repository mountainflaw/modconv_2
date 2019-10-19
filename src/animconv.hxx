struct MipsDataPointer {
    std::string name;
    long unsigned int offset;
};

struct AnimHeader {
    s16 flags;
    s16 unk02;
    s16 startFrame;
    s16 loopStart;
    s16 loopLength;
    s16 unused0A;
    s16 *values;
    s16 *index;
    s16 unusedLength;
};

void animconv_main(const std::string &file, const std::string &fileOut, bool level, struct AnimconvParams *params);
