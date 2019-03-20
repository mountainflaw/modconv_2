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

#include "main.hxx"

#include "f3d/vertex.hxx"
#include "f3d/displaylist.hxx"
#include "collision/prototypes.hxx"
#include "goddard/prototypes.hxx"

void error_message(std::string message)
{
    std::cout << "\e[ERROR]\e " << message << std::endl;
    exit(1);
}

void warn_message(std::string message)
{
    std::cout << "\e[WARNING]\e " << message << std::endl;
    return;
}

void info_message(std::string message)
{
    std::cout << "\e[INFO]\e " << message << std::endl;
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
            infoMessage("No output provided. Using 'model'.");
            break;
        case 3:
            filePath =        argv[1];
            fileOut  =        argv[2];
            infoMessage("No scale provided. Using default scale size.");
            break;
        case 4:
            filePath =        argv[1];
            fileOut  =        argv[2];
            scale = std::stoi(argv[3]);
            infoMessage("No microcode provided. Using Fast3D.");
            break;
        case 5:
            filePath =        argv[1];
            fileOut  =        argv[2];
            scale = std::stoi(argv[3]);
            ucode    =        argv[4];
        default:
            errorMessage("Syntax error.");
            break;
    }

    if (ucode.compare("f3d") == 0)              output = F3D;
    else if (ucode.compare("f3dex") == 0)       output = F3DEX;
    else if (ucode.compare("rej") == 0)         output = REJ;
    else if (ucode.compare("goddard") == 0)     output = GODDARD;
    else if (ucode.compare("collision") == 0)   output = COLLISION;
    else errorMessage("Invalid output type.");

    infoMessage("Starting...");
    prepareVertices(filePath, fileOut, scale, output); /* Starts construction process */
    infoMessage("Finished!");
    return 0;
}
