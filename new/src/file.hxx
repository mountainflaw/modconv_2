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

#include "../deps/lodepng.h"
#include "../deps/exoquant.h"

/* These includes aren't needed anymore due to the shift to C++17. */
/*
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif
*/

/*
 * Easily accessible functions to accomplish the following tasks:
 * 1.) Reset a file (Creating is done on a per file basis with fstreams)
 * 2.) Create and reset a directory
 * 3.) Get PNG information
 * 4.) Remove filename from path
 */

/**
 * Uses fstreams to reset a plaintext file.
 * Note: Adds the obsidian signature to line 1.
 */

void reset_file(const std::string &fileOut);

/** Deletes a directory and creates it again. */
void reset_directory(const std::string &output);

/** Uses LodePNG to get dimensions from a PNG image. */
u16 get_dimension(u8 mode, const std::string &path);

/** Easy way to get a path leading to the directory only. */
std::string get_path(const std::string &path);

/** Checks if a exists (including directories). */
bool file_exists(const std::string &path);

/** Checks if path provided is a directory. */
bool is_directory(const std::string &path);

/** Returns file name. */
std::string get_filename(const std::string &path);

/** Sanitize string for gas. */
std::string sanitize_output(const std::string &input);

/** Initialize output directory. */
void f3d_init_directory(const std::string &fileOut, u8 area);
