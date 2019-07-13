# modconv 3.0

This project aims to provide a usable model converter suitable for creating custom content compatible with the decompilation of Super Mario 64.

## Usage

### Building

To build modconv, you must have the ASSIMP library installed in addition to a compiler which supports the C++ 17 standard. The latter requires GCC 8 minimum (``g++-8`` is the package on Ubuntu versions that ship with GCC 7), unless you want to modify the source to use the experimental support in GCC 7. Sorry in advance, I was too lazy to write my own filesystem functions.

To build the default release version, simply run ``./build.sh``. To build the debug version to report errors, run ``./build.sh debug``.

modconv is able to be built on Windows using MSYS2, though has limited support. The build instructions are the same.

### Using

``./modconv <flags> path/to/model``

modconv accepts the following flags:

* ``--dir`` - The output directory.
* ``--level`` - Export as level (defaults to actor otherwise).
* ``--scale`` - Multiplier to scale the imported model by.
* ``--type`` - What to export. (f3d, f3dex, rej (64 vtx buffer), goddard (Mario head), collision).
* ``--glabel`` - All F3D symbols are exposed to the linker, and a C header file is created externing vertex data.
* ``--noscalehack`` - Disables the scaling hack introduced to fix the output by the matrix transformation.
* ``--fog rgba near far`` - Enables fog with the given parameters.
* ``--amb rgb`` - Sets ambient lighting color.
* ``--dif rgb xyz`` - Sets diffuse lighting color.
* ``--leniencyfactor`` - Sets the triangle optimization leniency factor. Defaults to 1.

modconv allows editing of several Fast3D material properties via placing parameters in material names ("grouptags"). The current grouptags are:

* ``#LIGHTING`` - Enables F3D vector lighting.
* ``#ENVMAP`` - Enables environment mapping.
* ``#LIN_ENVMAP`` - Enables environment mapping (mirrorball).
* ``#BACKFACE`` - Disables backface culling (Unlike all other materials, this may be enabled with others).
* ``#MIRROR(U | V)`` - Enables mirroring on the (U | V) axis.
* ``#CLAMP(U | V)`` - Enables clamping on the (U | V) axis.
* ``#NORMCOLORS`` - Use vertex normals as vertex colors.
* ``#REDALPHA`` - Copy the red vertex color channel into the alpha color channel.
* ``#LAYER_(0-7)`` - Splits display list into layer 0-7. Sets rendermode if fog is enabled.

In collision mode, setting surfaces is accomplished by inserting a ``!``, followed by the surface name or ID with no space inbetween, terminated by a space at the end, unless it is the end of the material name.

Example: ``material !SURF_ENV_DEFAULT #ENVMAP #LIGHTING``

## Roadmap

* ~~Full material class (CI support)~~
* ~~Optimizer~~
* ~~Collision~~
* Mario head

## Credits

* Trenavix, Robinerd, Davideesk for answering stupid F3D questions
* Davideesk again for cleaning up the F3D macros (especially LoadTextureBlock!)
* Kaze and stomatol for material ideas
* Revo for testing
* Jannik for playing anime music
