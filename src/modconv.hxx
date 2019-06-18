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

#pragma once
/********************************************************************************
 * FILE INCLUDES                                                                *
 ********************************************************************************/

#include <stdio.h>
#include <iostream>
#include <fstream>

#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>

#include <math.h>

#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

/********************************************************************************
 * TYPE DEFINITIONS                                                             *
 ********************************************************************************/

typedef signed char            s8;
typedef unsigned char          u8;
typedef signed short int       s16;
typedef unsigned short int     u16;
typedef signed int             s32;
typedef unsigned int           u32;
typedef signed long long int   s64;
typedef unsigned long long int u64;

typedef volatile u8   vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile u64 vu64;
typedef volatile s8   vs8;
typedef volatile s16 vs16;
typedef volatile s32 vs32;
typedef volatile s64 vs64;

typedef float  f32;
typedef double f64;

/* Workaround so file.hxx doesn't error out. */
#include "file.hxx"

/********************************************************************************
 * DEFINES                                                                      *
 ********************************************************************************/

#define DEFAULT_SCALE 350
/* Main GFX outputs */

/* 
 * These serve two purposes:
 * 1.) They inform the program what type of output to use and how to use it.
 * 2.) They define the vertex buffer limits for the graphics microcodes.
 */

#define OUTPUT_F3D       15
#define OUTPUT_F3DEX     30
#define OUTPUT_REJ       63
#define OUTPUT_REJ2      78 /* F3DLP Rej */

/* Other outputs */

#define OUTPUT_T3D       77 /* Unused */
#define OUTPUT_COLLISION 88
#define OUTPUT_GODDARD   99

/********************************************************************************
 * ENUMS                                                                        *
 ********************************************************************************/

enum axis { AXIS_X, AXIS_Y, AXIS_Z };

/********************************************************************************
 * PROTOTYPES                                                                   *
 ********************************************************************************/

void error_message(const std::string &message);
void warn_message(const std::string &message);
void info_message(const std::string &message);
std::string labelize(const std::string &label);

void collision_converter_main(const std::string &file, const std::string &fileOut, s16 scale);
