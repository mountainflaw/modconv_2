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

/* To decomp server members: constructive crticism is fine, but being a smartass is not */

#include "include.hxx"

void errorMessage(int errorCode, std::string info)
{
    switch(errorCode)
    {
        case ERROR_SYNTAX:
            std::cerr << "\e[1m[ERROR]\e[0m Invalid syntax." << std::endl;
            std::cerr << "\e[1m[INFO]\e[0m Usage: " << info << " <path to model> <output name and directory> <scale> <microcode>" << std::endl;
            std::cerr << "\e[1m[INFO]\e[0m Defaults: <path to model> model 350 f3d" << std::endl;
            std::cerr << "\e[1m[INFO]\e[0m <microcode> is able to be set to 'f3d', 'f3dex', and 'rej'." << std::endl;
            break;
        case ERROR_CANT_READ:
            std::cerr << "\e[1m[ERROR]\e[0m Cannot access " << info << " . Does it exist?" << std::endl;
            break;
        case ERROR_CANT_WRITE:
            std::cerr << "\e[1m[ERROR]\e[0m Cannot write to " << info << " . Do you have permission to write to it?" << std::endl;
            break;
        case ERROR_NO_TYPE:
            std::cerr << "\e[1m[ERROR]\e[0m Invalid output type." << std::endl;

    }
    exit(1);
}

void warnMessage(int warnCode, int arg1, int arg2, int arg3, int arg4)
{
    switch(warnCode)
    {
        case WARNING_BIG_TRIANGLE: /* Warn for large triangles. arg3 and arg4 go unused */
            printf("\e[1m[WARNING]\e[0m Triangle %d has an area of %d. This can potentionally produce\
            graphical oddities on real hardware and accurate RDP emulation setups (CEN64 / Angrylion's.\n", arg1, arg2);
        break;
        case WARNING_LARGE_POLYCOUNT:
            printf("\e[1m[WARNING]\e[0m This mesh has %d polygons. It may be laggy on real hardware.\n", arg1);
            break;
        case WARNING_LARGE_VERTCOUNT:
            printf("\e[1m[WARNING]\e[0m This mesh has %d vertices. It may be laggy on real hardware.\n", arg1);
            break;
        
    }
    return;
}

void infoMessage(int infoCode)
{
    switch(infoCode)
    {
        case INFO_NO_OUTPUT:
            puts("\e[1m[INFO]\e[0m No arguments provided for output directory name, using 'model'.");
        case INFO_NO_SCALE:
            printf("\e[1m[INFO]\e[0m No arguments provided for scale, using %d.\n", DEFAULT_SCALE);
        case INFO_NO_MICROCODE:
            puts("\e[1m[INFO]\e[0m No arguments provided for microcode type, using 'f3d'.");
            break;
        case INFO_PREPARING_VTX:
            puts("\e[1m[INFO]\e[0m Preparing vertices.");
            break;
        case INFO_PREPARING_DL:
            puts("\e[1m[INFO]\e[0m Preparing display list.");
            break;
        case INFO_BUILDING_VTX:
            puts("\e[1m[INFO]\e[0m Building vertices.");
            break;
        case INFO_BUILDING_DL:
            puts("\e[1m[INFO]\e[0m Building displaylist.");
            break;
        case INFO_STARTED:
            puts("\e[1m[INFO]\e[0m Starting...");
            break;
        case INFO_FINISHED:
            puts("\e[1m[INFO]\e[0m Finished!");
            break;
        case INFO_NOT_IMPLEMENTED_OPTIMIZER:
            puts("\e[1m[INFO]\e[0m The optimizer is not yet implemented."); /* UNUSED */
            break;
        case INFO_FINISHED_VTX:
            puts("\e[1m[INFO]\e[0m Finished building vertices.");
            break;
        case INFO_FINISHED_DL:
            puts("\e[1m[INFO]\e[0m Finished building display list.");
            break;
    }
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
            infoMessage(INFO_NO_OUTPUT);
            break;
        case 3:
            filePath =        argv[1];
            fileOut  =        argv[2];
            infoMessage(INFO_NO_SCALE);
            break;
        case 4:
            filePath =        argv[1];
            fileOut  =        argv[2];
            scale = std::stoi(argv[3]);
            infoMessage(INFO_NO_MICROCODE);
            break;
        case 5:
            filePath =        argv[1];
            fileOut  =        argv[2];
            scale = std::stoi(argv[3]);
            ucode    =        argv[4];
        default:
            errorMessage(ERROR_SYNTAX, argv[0]);
            break;
    }

    if (ucode.compare("f3d") == 0)              output = F3D;
    else if (ucode.compare("f3dex") == 0)       output = F3DEX;
    else if (ucode.compare("rej") == 0)         output = REJ;
    else if (ucode.compare("goddard") == 0)     output = GODDARD;
    else if (ucode.compare("collision") == 0)   output = COLLISION;
    else errorMessage(ERROR_NO_TYPE, "");

    infoMessage(INFO_STARTED);
    prepareVertices(filePath, fileOut, scale, output); /* Starts construction process */
    infoMessage(INFO_FINISHED);
    return 0;
}
