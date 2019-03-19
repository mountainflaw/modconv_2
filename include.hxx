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

/* This file contains constant definitions, prototypes, and includes */

#ifndef _INCLUDEMAIN
#define _INCLUDEMAIN
#endif

#ifdef _INCLUDEMAIN

#include <stdio.h>
#include <iostream>
#include <fstream>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

/* Error messages */

#define ERROR_SYNTAX     0 /* Launch syntax invalid */
#define ERROR_CANT_READ  1 /* Cannot read file */
#define ERROR_CANT_WRITE 2 /* Cannot write to file */
#define ERROR_NO_TYPE    3 /* Invalid output */
#define ERROR_WONKY_FILE 4 /* Weird characters in file name */

/* Warning messages */

#define WARNING_BIG_TRIANGLE    0 /* When the triangle's area is dangerously large */
#define WARNING_LARGE_POLYCOUNT 1 /* When the poly count is dangerously large */
#define WARNING_LARGE_VERTCOUNT 2 /* When the vertex count is dangerously latge */

/* Info  messages */

#define INFO_NO_OUTPUT                  0 /* Default */
#define INFO_NO_SCALE                   1 /* Default */
#define INFO_NO_MICROCODE               2 /* Default */
#define INFO_PREPARING_VTX              3 /* Running checks on now acquired vertex data */
#define INFO_PREPARING_DL               4 /* Running checks on inputted vertex data */
#define INFO_BUILDING_VTX               5 /* Vertex writing started */
#define INFO_BUILDING_DL                6 /* Display list writing started */
#define INFO_STARTED                    7 /* Execution started */
#define INFO_FINISHED                   8 /* Execution has finished */
#define INFO_NOT_IMPLEMENTED_OPTIMIZER  9 /* UNUSED: Vertex optimizer not implemented */
#define INFO_FINISHED_VTX              10 /* Finished building vertices */
#define INFO_FINISHED_DL               11 /* Finished building display list */

/*******************/
/* Other constants */
/*******************/

#define DEFAULT_SCALE 350

/* Output mode types */

#define F3D 15
#define F3DEX 30
#define REJ 63
#define GODDARD 64
#define COLLISION 65

/* UV mapping */

#define AXIS_U 0
#define AXIS_V 1

/* Vertex positions */

#define AXIS_X 0
#define AXIS_Y 1
#define AXIS_Z 2

/* Vertex RGBA color channels */

#define CHANNEL_RED 0
#define CHANNEL_BLUE 1
#define CHANNEL_GREEN 2
#define CHANNEL_ALPHA 3

/* Miscellaneous */

#define WARNING_VERTS_NUMBER 4500
#define VTX_DONT_SKIP        -1 /* Signals the display list builder to not skip this vertex */
#define VTX_NORMAL_NONE      -1 /* Signals the display list builder that there are no normals */

/* Prototype what we need from main.cxx */

void errorMessage(int errorCode, std::string info);
void warnMessage(int warnCode, int arg1, int arg2, int arg3, int arg4);
void infoMessage(int infoCode);

/* Prototype what we need from vertex.cxx */

void prepareVertices(std::string file, std::string fileOut, int scale, int f3d);

/* Prototype what we need from displaylist.cxx */

void prepareDisplayList(std::string fileOut, struct vertex *vtx, int f3d, int tVerts, std::string *mats);

struct vertex
{
    int       vertPos[3];
    int       uv[2];
    int       rgba[4];
    int       norm;    /* Used for culling   */
    int       map;     /* Used for optimizer */
    int       oldmap;  /* Old pre-optimizer order */
    int       newmap;  /* New post-optimizer order */
    int       mesh[2]; /* Used for materials. Index 0 and 1 are used for mesh # and wrapping mode respectively */
};


/* no */

/*class vertex
{
    public:
    vertex() {}; 

    unsigned short GetPosition(int axis)
    {
        return vertPos[axis];
    }

    unsigned short GetRgba(int axis)
    {
        return rgba[axis];
    }

    unsigned short GetUv(int axis)
    {
        return uv[axis];
    }

    void SetPosition(int axis, unsigned short pos)
    {
        vertPos[axis] = pos;
    }

    void SetUv(int axis, unsigned short pos)
    {
        uv[axis] = pos;
    }

    void SetColors(int axis, char value)
    {
        rgba[axis] = value;
    }

    private:
    int vertPos[3];
    int      uv[2];
    int    rgba[3];
};*/

#endif
