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
enum color { C_RED, C_GREEN, C_BLUE, C_ALPHA };

/* MATERIALS */

enum matTypes 
{
    SOLID_COLOR, 
    TEXTURED, 
    ENV_MAPPED, 
    ALPHA_BLEND,
    DEFAULT
};

enum wrappingModes { TEXWRAP, TEXCLAMP, TEXUNUSED, TEXMIRROR };

enum texTypes { RGBA32, RGBA16, IA16, CI8, IA8, I8, CI4, IA4, I4 };
enum geoTypes { TEXGEN, BKFACE, TBS0, TBS1, TBS2, TBS3, TBS4, TBS5 };

/********************************************************************************
 * PROTOTYPES                                                                   *
 ********************************************************************************/

void error_message(const std::string &message);
void warn_message(const std::string &message);
void info_message(const std::string &message);

void collision_converter_main(const std::string &file, const std::string &fileOut, s16 scale);
