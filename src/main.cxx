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
 *       * Neither the name of the Obsidian developers nor the                           *
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

#include "modconv.hxx"

/* glabel mode */
bool glabel = false;
/* std::string glabelData; */

void f3d_main(const std::string &file, const std::string &fileOut, s16 scale, u8 microcode, bool level, bool yUp, bool uvFlip);
void collision_converter_main(const std::string &file, const std::string &fileOut, s16 scale, bool yUp);

u8 output = OUTPUT_F3D;

inline std::string print_bold(const std::string &s)
{
#ifdef _WIN32
    return s;
#else
    return "\e[1m" + s + "\e[0m";
#endif
}

inline void error_message(const std::string &message)
{
    std::cout << print_bold("ERROR: ") << message << std::endl;
    exit(1);
}

inline void warn_message(const std::string &message)
{ std::cout << print_bold("WARNING: ") << message << std::endl; }

inline void info_message(const std::string &message)
{ std::cout << print_bold("INFO: ") << message << std::endl; }

void print_help(const std::string &name)
{
    std::cout << print_bold("- MODCONV 2.8 HELP -") << std::endl;
    std::cout << std::endl;
    std::cout << print_bold("SYNOPSIS: ") << std::endl;
    std::cout << name << " <parameters> <model>" << std::endl;
    std::cout << std::endl;
    std::cout << print_bold("PARAMETERS: ") << std::endl;
    std::cout << "--level - Export as a level" << std::endl;
    std::cout << "--dir   - Output directory" << std::endl;
    std::cout << "--scale - Multiplier to scale the exported model by" << std::endl;
    std::cout << "--type  - What to export" << std::endl;
    std::cout << "  - f3d       - Use Fast3D (default)" << std::endl;
    std::cout << "  - f3dex     - Use Fast3DEX (requires f3dex to be enabled)" << std::endl;
    std::cout << "  - rej       - Optimize for Fast3DEX Rej (64 vtx)" << std::endl;
    std::cout << "  - collision - Export collision mesh" << std::endl;
    std::cout << "  - goddard   - Export Mario head mesh" << std::endl;
    std::cout << "--yup    - Use the Y axis for up" << std::endl;
    std::cout << "  - Acceptable inputs are Y and Z." << std::endl;
    std::cout << "--uvflip - Flip the UV mask Y axis." << std::endl;
    std::cout << "--glabel - Use global labels instead of local labels." << std::endl;
    std::cout << "  - Allows for editing data in C." << std::endl;
    std::cout << "--help   - Bring up this menu and quit" << std::endl;
    std::cout << std::endl;
    std::cout << print_bold("TIPS: ") << std::endl;
    std::cout << "- If you use obj, set up direction to the Y axis using --up" << std::endl;
    std::cout << "- If your model lags, consider using Fast3DEX (--type f3dex)" << std::endl;
    std::cout << "- If on console and or angrylion's you experience triangle warping, subdivide the problematic triangle" << std::endl;
}

/* GCC seems to think that the level bool is unused... */
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

/** Used to generate assembler labels so we can easily use glabel mode. */
std::string labelize(const std::string &label)
{
    if (glabel) {
        return "glabel " + label;
    }

    else { /* Regular labels (default behavior) */
        return label + ":";
    }
}

int main(int argc, char* argv[])
{
    std::cout << print_bold("- MODCONV 2.8 BY RED -") << std::endl;
    std::string filePath = argv[argc - 1],
                fileOut  = "model";
    s16 scale            = DEFAULT_SCALE;
    bool level           = false,
         yUp             = false,
         uvFlip          = false;

    if (argc < 2) {
        print_help(argv[0]);
        exit(0);
    }

    /* Parse arguments */
    for (int i = 1; i < argc - 1; i++) {
        std::string arg = argv[i],
                    flw = argv[i + 1];

        if (arg.compare("--level") == 0) {
            std::cout << "DBG - Level mode enabled" << std::endl;
            level = true;
        }

        if (arg.compare("--dir") == 0) {
            std::cout << "DBG - Output is " << flw << std::endl;
            fileOut = flw;
        }

        if (arg.compare("--scale") == 0) {
            scale = std::stoi(flw);
            std::cout << "DBG - Scale: " << scale << std::endl;
        }

        if (arg.compare("--type") == 0) {
            std::cout << "DBG - Type: " << flw << std::endl;

            if (flw.compare("f3d") == 0)              output = OUTPUT_F3D;
            else if (flw.compare("f3dex") == 0)       output = OUTPUT_F3DEX;
            else if (flw.compare("rej") == 0)         output = OUTPUT_REJ;
            else if (flw.compare("goddard") == 0)     output = OUTPUT_GODDARD;
            else if (flw.compare("collision") == 0)   output = OUTPUT_COLLISION;
            else error_message("Invalid output type.");
        }

        if (arg.compare("--yup") == 0) {
            yUp = true;
        }

        if (arg.compare("--uvflip") == 0) {
            uvFlip = true;
        }

        if (arg.compare("--glabel") == 0) {
            glabel = true;
        }

        if (arg.compare("--help") == 0) {
            print_help(argv[0]);
            exit(0);
        }
    }

    if (filePath.compare("--help") == 0) { /* If the only argument is help, bring up help instead of converting. */
        print_help(argv[0]);
        exit(0);
    }

    if (!(file_exists(filePath))) {
        error_message(filePath + " does not exist.");
    }

    std::cout << "DBG - Args: " << argc << std::endl;
    info_message("Starting...");

    reset_directory(fileOut);
    switch (output) {
        case OUTPUT_F3D:
        case OUTPUT_F3DEX:
        case OUTPUT_REJ:
        case OUTPUT_REJ2:
        f3d_main(filePath, fileOut, scale, output, level, yUp, uvFlip);
        break;

        case OUTPUT_COLLISION:
        collision_converter_main(filePath, fileOut, scale, yUp);
        break;

    }

    if (glabel) {
        info_message("Exporting C header...");
    }

    info_message("Finished!");
    return 0;
}
