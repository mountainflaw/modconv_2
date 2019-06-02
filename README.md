# modconv 2.8

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


## Planned Differences

Goals have been slightly extended, as I've been asked to include a couple of material presets which you can activate
with custom material names. The planned materials are:

* ``#ENVMAP`` - Enables environment mapping
* ``#ENVMAP2`` - Enables environment mapping (mirrorball)
* ``#BACKFACE`` - Disables backface culling (Unlike all other materials, this may be enabled with others)

Flat shaded polygons are supported by assigning a materal that does not use a texture and are setup with primcolor.

For texture mapping, I will be supporting every texture that is not YUV, with automatically generated palettes.

## Roadmap

* ~~Full material class (CI support)~~
* ~~Optimizer~~
* ~~Collision~~
* Mario head

Optional:

* Split mesh up by material

## Credits

* Trenavix, Robinerd, Davideesk for answering stupid F3D questions
* Davideesk again for cleaning up the F3D macros (especially LoadTextureBlock!)
* Kaze and stomatol for material ideas
* Revo for testing.
