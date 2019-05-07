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

#include "common.hxx"

void vtx_phase(const std::string &file, const std::string &fileOut, s16 scale, u8 f3d, u8 area);

void error_message(const std::string &message)
{
    std::cout << "\e[1mERROR:\e[0m " << message << std::endl;
    exit(1);
}

void warn_message(const std::string &message)
{
    std::cout << "\e[1mWARNING:\e[0m " << message << std::endl;
}

void info_message(const std::string &message)
{
    std::cout << "\e[1mINFO:\e[0m " << message << std::endl;
}

void print_help(const std::string &name)
{
    std::cout << "-\e[1m modconv 2 help -\e[0m" << std::endl;
    std::cout << std::endl;
    std::cout << "\e[1mSYNOPSIS:\e[0m" << std::endl;
    std::cout << name << " <parameters> <model>" << std::endl;
    std::cout << std::endl;
    std::cout << "\e[1mPARAMETERS:\e[0m" << std::endl;
    std::cout << "--level - Export as a level" << std::endl;
    std::cout << "--dir   - Output directory" << std::endl;
    std::cout << "--scale - Multiplier to scale the exported model by" << std::endl;
    std::cout << "--type  - What to export" << std::endl;
    std::cout << "  - f3d       - Use Fast3D (default)" << std::endl;
    std::cout << "  - f3dex     - Use Fast3DEX (requires f3dex to be enabled)" << std::endl;
    std::cout << "  - rej       - Optimize for Fast3DEX Rej (64 vtx)" << std::endl;
    std::cout << "  - rej2      - Optimize for Fast3DLP Rej (80 vtx)" << std::endl;
    std::cout << "  - collision - Export collision mesh" << std::endl;
    std::cout << "  - goddard   - Export Mario head mesh" << std::endl;
    std::cout << "--up    - Direction for the up axis (Defaults to Z)" << std::endl;
    std::cout << "--help  - Bring up this menu and quit" << std::endl;
    std::cout << std::endl;
    std::cout << "\e[1mTIPS:\e[0m" << std::endl;
    std::cout << "- If you use SketchUp, set up direction to the Y axis using --up" << std::endl;
    std::cout << "- If your model lags, consider using Fast3DEX (--f3dex)" << std::endl;
    std::cout << "- If on console and or angrylion's you experience triangle warping, subdivide the problematic triangle" << std::endl;
}

/* GCC seems to think that the level bool is unused... */
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

int main(int argc, char* argv[])
{
    puts("\e[1m- modconv 2.6 by red -\e[0m");
    std::string filePath = argv[argc - 1],
                fileOut  = "model";
    s16 scale            = DEFAULT_SCALE;
    u8 output            = OUTPUT_F3D,
       axis              = AXIS_Z,
       area              = 0;

    if (argc < 2)
    {
        print_help(argv[0]);
        error_message("Too few arguments.");
    }

    /* Parse arguments */
    for (int i = 1; i < argc - 1; i++)
    {
        std::string arg = argv[i],
                    flw = argv[i + 1];

        if (arg.compare("--level") == 0)
        {
            std::cout << "DBG - Level mode enabled" << std::endl;
            area = 1;
        }

        if (arg.compare("--dir") == 0)
        {
            std::cout << "DBG - Output is " << flw << std::endl;
            fileOut = flw;
        }

        if (arg.compare("--scale") == 0)
        {
            scale = std::stoi(flw);
            std::cout << "DBG - Scale: " << scale << std::endl;
        }

        if (arg.compare("--type") == 0)
        {
            std::cout << "DBG - Type: " << flw << std::endl;

            if (flw.compare("f3d") == 0)              output = OUTPUT_F3D;
            else if (flw.compare("f3dex") == 0)       output = OUTPUT_F3DEX;
            else if (flw.compare("rej") == 0)         output = OUTPUT_REJ;
            else if (flw.compare("rej2") == 0)        output = OUTPUT_REJ2;
            else if (flw.compare("goddard") == 0)     output = OUTPUT_GODDARD;
            else if (flw.compare("collision") == 0)   output = OUTPUT_COLLISION;
            else error_message("Invalid output type.");
        }

        if (arg.compare("--up") == 0)
        {
            if (flw.compare("x") == 0 || flw.compare("X") == 0)      axis = AXIS_X;
            else if (flw.compare("y") == 0 || flw.compare("Y") == 0) axis = AXIS_Y;
            else if (flw.compare("z") == 0 || flw.compare("Z") == 0) axis = AXIS_Z;
            else error_message("Invalid up direction.");
        }

        if (arg.compare("--help") == 0)
        {
            print_help(argv[0]);
            exit(0);
        }
    }

    if (filePath.compare("--help") == 0) /* If the only argument is help, bring up help instead of converting. */
    {
        print_help(argv[0]);
        exit(0);
    }

    if (!(file_exists(filePath)))
        error_message(filePath + " does not exist.");

    std::cout << "DBG - Args: " << argc << std::endl;
    info_message("Starting...");

    if (output == OUTPUT_COLLISION)
        collision_converter_main(filePath, fileOut, scale);
    else
    {
        f3d_init_directory(fileOut, area);
        vtx_phase(filePath, fileOut, scale, output, area); /* Starts construction process */
    }

    info_message("Finished!");
    return 0;
}
