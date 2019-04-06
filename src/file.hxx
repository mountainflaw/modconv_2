#pragma once
#include <filesystem>
#include "../deps/lodepng.h"

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
int get_dimension(int mode, const std::string &path);

/** Easy way to get a path leading to the directory only */
std::string get_path(const std::string &path);

/** Checks if a exists (including directories) */
bool file_exists(const std::string &path);

/** Checks if path provided is a directory */
bool is_directory(const std::string &path);
