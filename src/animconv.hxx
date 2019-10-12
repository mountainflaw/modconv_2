struct MipsDataPointer {
    std::string name;
    long unsigned int offset;
};

void animconv_main(const std::string &file, const std::string &fileOut, bool level, struct AnimconvParams *params);
