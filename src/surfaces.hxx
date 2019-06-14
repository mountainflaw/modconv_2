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

#include "modconv.hxx"

#define SURFACES 57

std::string surfaces[] =
{
    "!SURF_ENV_DEFAULT",        "!SURF_BURNING",        "!SURF_04",
    "!SURF_CEILING",            "!SURF_SLOW",           "!SURF_DEATH_FLOOR",
    "!SURF_CLOSE_CAM",          "!SURF_WATER",          "!SURF_WATER_FLOW",
    "!SURF_INTANGIBLE",         "!SURF_SLIPPERY",       "!SURF_SLIGHT_SLIP",
    "!SURF_NO_SLIP",            "!SURF_VARIED_NOISE",   "!SURF_INST_WARP_1B",
    "!SURF_INST_WARP_1C",       "!SURF_INST_WARP_1D",   "!SURF_INST_WARP_1E",
    "!SURF_SAND_21",            "!SURF_QUICKSAND_22",   "!SURF_QUICKSAND_23",
    "!SURF_QUICKSAND_24",       "!SURF_QUICKSAND_25",   "!SURF_QUICKSAND_26",
    "!SURF_QUICKSAND_27",       "!SURF_WALL_ETC",       "!SURF_NOISE_DEF",
    "!SURF_NOISE_SLIP",         "!SURF_WIND",           "!SURF_QUICKSAND_2D",
    "!SURF_SLIP_ICE",           "!SURF_LOOK_WARP",      "!SURF_HARD_FLOOR",
    "!SURF_WARP",               "!SURF_TIMER_ON",       "!SURF_TIMER_OFF",
    "!SURF_HARD_SLIP",          "!SURF_HARD",           "!SURF_NO_SLIP_ICE",
    "!SURF_DEATH_WIND",         "!SURF_WIDE_CAM",       "!SURF_WALLS_66",
    "!SURF_PYR_TOP_BOOM",       "!SURF_CAM_6F",         "!SURF_CAM_70",
    "!SURF_NO_ACCESS",          "!SURF_NOISE_SLD_73",   "!SURF_NOISE_SLD_74",
    "!SURF_CAM_75",             "!SURF_FLAG_SURF_76",   "!SURF_FLAG_SURF_77",
    "!SURF_UNK_NOISE",          "!SURF_SLIPPERY_79",    "!SURF_ACTIVATE",
    "!SURF_VAN_CAP_WALL",       "!SURF_POOL_WARP",      "!SURF_TRAPDOOR"
};
