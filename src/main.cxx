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

//#include "f3d/vertex.hxx"
//#include "f3d/displaylist.hxx"
//#include "collision/prototypes.hxx"
//#include "goddard/prototypes.hxx"

void error_message(const std::string &message)
{
    std::cout << "\e[1mERROR:\e[0m " << message << std::endl;
    exit(1);
}

void warn_message(const std::string &message)
{
    std::cout << "\e[1mWARNING:\e[0m " << message << std::endl;
    return;
}

void info_message(const std::string &message)
{
    std::cout << "\e[1mINFO:\e[0m " << message << std::endl;
    return;
}

int main(int argc, char* argv[])
{
    puts("\e[1m- obsidian by red -\e[0m");
    std::string filePath,
                fileOut = "model",
                ucode   =   "f3d";
    int output, scale   = DEFAULT_SCALE;
    switch(argc)
    {
        case 2:
            filePath =        argv[1];
            info_message("No output provided. Using 'model'.");
            break;
        case 3:
            filePath =        argv[1];
            fileOut  =        argv[2];
            info_message("No scale provided. Using default scale size.");
            break;
        case 4:
            filePath =        argv[1];
            fileOut  =        argv[2];
            scale = std::stoi(argv[3]);
            info_message("No microcode provided. Using Fast3D.");
            break;
        case 5:
            filePath =        argv[1];
            fileOut  =        argv[2];
            scale = std::stoi(argv[3]);
            ucode    =        argv[4];
            break;
        default:
            error_message("Syntax error.");
            break;
    }

    if (ucode.compare("f3d") == 0)              output = OUTPUT_F3D;
    else if (ucode.compare("f3dex") == 0)       output = OUTPUT_F3DEX;
    else if (ucode.compare("rej") == 0)         output = OUTPUT_REJ;
    else if (ucode.compare("rej2") == 0)        output = OUTPUT_REJ2;
    else if (ucode.compare("goddard") == 0)     output = OUTPUT_GODDARD;
    else if (ucode.compare("collision") == 0)   output = OUTPUT_COLLISION;
    else error_message("Invalid output type.");
    std::cout << "args: " << argc << std::endl;
    void vertex_phase(const std::string &file, const std::string &fileOut, s16 scale, s8 f3d);
    info_message("Starting...");
    vertex_phase(filePath, fileOut, scale, output); /* Starts construction process */
    info_message("Finished!");
    return 0;
}
