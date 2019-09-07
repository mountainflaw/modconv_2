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

#include "modconv.hxx"

bool glabel = false;
bool scalingHack = true;
bool gUvFlip = false;
bool gExportC = false;
bool gGeneric = false;
u8 leniencyFactor = 1;
/* std::string glabelData; */

u8 output = OUTPUT_F3D;

INLINE std::string print_bold(const std::string &s) {
#ifdef _WIN32
    return s;
#else
    return "\e[1m" + s + "\e[0m";
#endif
}

INLINE void error_message(const std::string &message) {
    std::cout << print_bold("ERROR: ") << message << std::endl;
    exit(1);
}

void warn_message(const std::string &message)
{ std::cout << print_bold("WARNING: ") << message << std::endl; }

INLINE void info_message(const std::string &message)
{ std::cout << print_bold("INFO: ") << message << std::endl; }

void print_help(const std::string &name) {
    std::cout << print_bold("- MODCONV 3.5 HELP -") << std::endl
              << std::endl
              << print_bold("SYNOPSIS: ") << std::endl
              << name << " <parameters> <model>" << std::endl
              << std::endl
              << print_bold("PARAMETERS: ") << std::endl
              << "--level - Export as a level" << std::endl
              << "--dir   - Output directory" << std::endl
              << "--scale - Multiplier to scale the exported model by" << std::endl
              << "--type  - What to export" << std::endl
              << "  - f3d       - Use Fast3D (default)" << std::endl
              << "  - f3dex     - Use Fast3DEX (requires f3dex to be enabled)" << std::endl
              << "  - rej       - Optimize for Fast3DEX Rej (64 vtx)" << std::endl
              << "  - collision - Export collision mesh" << std::endl
              << "  - goddard   - Export Mario head mesh" << std::endl
              << "--uvflip - Flip the UV mask Y axis" << std::endl
              << "--glabel - Use global labels instead of local labels" << std::endl
              << "  - Allows for editing data in C" << std::endl
              << "--dif    - Changes the diffuse lighting color" << std::endl
              << "  - r         - Amount of red" << std::endl
              << "  - g         - Amount of green" << std::endl
              << "  - b         - Amount of blue" << std::endl
              << "  - x         - X lighting direction" << std::endl
              << "  - y         - Y lighting direction" << std::endl
              << "  - z         - Z lighting direction" << std::endl
              << "--amb    - Changes the ambient lighting color" << std::endl
              << "  - r         - Amount of red" << std::endl
              << "  - g         - Amount of green" << std::endl
              << "  - b         - Amount of blue" << std::endl
              << "--fog    - Enables fog and also sets fog color and distance" << std::endl
              << "  - r         - Amount of red" << std::endl
              << "  - g         - Amount of green" << std::endl
              << "  - b         - Amount of blue" << std::endl
              << "  - a         - Amount of alpha" << std::endl
              << "  - n         - Near amount of fog" << std::endl
              << "  - f         - Far amount of fog" << std::endl
              << "--noscalehack - Disable the scaling hack (disables multiplying scale by 0.01)" << std::endl
              << "--leniencyfactor - Sets the triangle optimization leniency factor. Defaults to 1." << std::endl
              << "--generic     - Export display list in generic format (contains rendermode sets)" << std::endl
              << "--export-c    - Exports display list data in C format instead of gas format." << std::endl
              << "--help   - Bring up this menu and quit" << std::endl
              << std::endl
              << print_bold("TIPS: ") << std::endl
              << "- If your model lags, consider using Fast3DEX (--type f3dex)" << std::endl
              << "- If on console and or angrylion's you experience triangle warping, subdivide the problematic triangle" << std::endl;
}

/* GCC seems to think that the level bool is unused... */
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

/** Used to generate assembler labels so we can easily use glabel mode. */
std::string labelize(const std::string &label) {
    if (glabel) {
        return "glabel " + label;
    } else { /* Regular labels (default behavior) */
        return label + ":";
    }
}

void extern_data(const std::string &fileOut, const std::string &a) {
    if (glabel) {
        std::fstream header;
        header.open(fileOut + "/" + fileOut + ".h", std::iostream::out | std::iostream::app);
        header << a << std::endl;
        header.close();
    }
}

/* FBX multiplies vertex positions by 100. We counter this by multiplying FBX models by 0.01. */
f32 scaling_hack() {
    if (scalingHack) {
        return 0.01f;
    } else {
        return 1.0f;
    }
}

int main(int argc, char* argv[]) {
    std::cout << print_bold("- MODCONV 3.5 BY RED -") << std::endl;
    std::string filePath = argv[argc - 1],
                fileOut  = "model";
    s16 scale            = DEFAULT_SCALE;
    bool level           = false;

    if (argc < 2) {
        print_help(argv[0]);
        exit(0);
    }

    /* Parse arguments */
    for (int i = 1; i < argc - 1; i++) {
        std::string arg = argv[i],
                    flw = argv[i + 1];

        if (arg.compare("--level") == 0) {
            //std::cout << "DBG - Level mode enabled" << std::endl;
            level = true;
        }

        if (arg.compare("--dir") == 0) {
            //std::cout << "DBG - Output is " << flw << std::endl;
            fileOut = flw;
        }

        if (arg.compare("--scale") == 0) {
            scale = std::stoi(flw);
            //std::cout << "DBG - Scale: " << scale << std::endl;
        }

        if (arg.compare("--type") == 0) {
            if (flw.compare("f3d") == 0) {            output = OUTPUT_F3D; }
            else if (flw.compare("f3dex") == 0) {     output = OUTPUT_F3DEX; }
            else if (flw.compare("rej") == 0) {       output = OUTPUT_REJ; }
            else if (flw.compare("goddard") == 0) {   output = OUTPUT_GODDARD; }
            else if (flw.compare("collision") == 0) { output = OUTPUT_COLLISION; }
            else if (flw.compare("animation") == 0) { output = OUTPUT_ANIMATION; }
            else { error_message("Invalid output type."); }
            info_message("Type: " + flw);
        }

        if (arg.compare("--uvflip") == 0) {
            gUvFlip = true;
        }

        if (arg.compare("--glabel") == 0) {
            glabel = true;
        }

        if (arg.compare("--dif") == 0) {
            if (i + 5 >= argc) {
                error_message("--dif requires five arguments.");
            }

            diffuse[3] = std::stoi(argv[i + 1]);
            diffuse[4] = std::stoi(argv[i + 2]);
            diffuse[5] = std::stoi(argv[i + 3]);
            diffuse[0] = std::stoi(argv[i + 4]);
            diffuse[1] = std::stoi(argv[i + 5]);
            diffuse[2] = std::stoi(argv[i + 6]);
        }

        if (arg.compare("--amb") == 0) {
            if (i + 3 >= argc) {
                error_message("--amb requires three arguments.");
            }

            ambient[0] = std::stoi(argv[i + 1]);
            ambient[1] = std::stoi(argv[i + 2]);
            ambient[2] = std::stoi(argv[i + 3]);
        }

        if (arg.compare("--fog") == 0) {
            if (i + 6 >= argc) {
                error_message("--fog requires six arguments.");
            }

            fogSettings[0] = std::stoi(argv[i + 1]);
            fogSettings[1] = std::stoi(argv[i + 2]);
            fogSettings[2] = std::stoi(argv[i + 3]);
            fogSettings[3] = std::stoi(argv[i + 4]);
            fogSettings[4] = std::stoi(argv[i + 5]);
            fogSettings[5] = std::stoi(argv[i + 6]);
            fog = true;
        }

        if (arg.compare("--leniencyfactor") == 0) {
            leniencyFactor = std::stoi(flw);
        }

        if (arg.compare("--noscalehack") == 0) {
            scalingHack = false;
        }

        if (arg.compare("--export-c") == 0) {
            gExportC = true;
        }

        if (arg.compare("--generic") == 0) {
            gGeneric = true;
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

    //std::cout << "DBG - Args: " << argc << std::endl;
    info_message("Starting...");

    reset_directory(fileOut);

    if (glabel) {
        info_message("C header will be exported as " + print_bold(fileOut + "/" + fileOut + ".h") + ".");
        reset_file(fileOut + "/" + fileOut + ".h");
        std::fstream header;
        header.open(fileOut + "/" + fileOut + ".h", std::iostream::out | std::iostream::app);
        header << std::endl;
        header.close();
    }

    switch (output) {
        case OUTPUT_F3D:
        case OUTPUT_F3DEX:
        case OUTPUT_REJ:
        case OUTPUT_REJ2:
        f3d_main(filePath, fileOut, scale, output, level);
        break;

        case OUTPUT_COLLISION:
        collision_converter_main(filePath, fileOut, scale);
        break;

        //case OUTPUT_GODDARD:
        //goddard_main(filePath, fileOut, scale);
        //break;

        case OUTPUT_ANIMATION:
        animconv_main(filePath, fileOut);
        break;
    }

    info_message("Finished!");
    return 0;
}
