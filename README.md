# Obsidian 2

The original project, same intentions but with a much much more flexible codebase.

## Planned Differences

Goals have been slightly extended, as I've been asked to include a couple of material presets which you can activate
with custom material names. The planned materials are:

* ``#ENVMAP`` - Enables environment mapping
* ``#BACKFACE`` - Disables backface culling (Unlike all other materials, this may be enabled with others)
* ``#MULTITEXTURE`` - Enables the use of blending two intensity textures for blended textures

Flat shaded polygons are supported by assigning a materal that does not use a texture and are setup with primcolor.

For texture mapping, I will be supporting every texture that is not YUV, with automatically generated palettes.

## Roadmap

* Full material class
* Optimizer
* Collision
* Mario head

Optional:

* Split mesh up by material

## Credits

* Trenavix, Robinerd, Davideesk for answering stupid F3D questions
* Kaze and stomatol for material ideas
