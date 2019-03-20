/********************************************************************************
 * DEFINES                                                                      *
 ********************************************************************************/

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
    HILITE_MAPPED, 
    ALPHA_BLEND 
};
