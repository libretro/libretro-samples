boxes
=====

OpenGL 4.3 experiment with compute shaders, and tons of instanced geometry! Targets [libretro](http://libretro.com) GL as a backend.

Purpose of the compute shader is to do frustum culling, LOD-sorting and some simple physics for fun.

After compute shader, indirect drawing is used to draw instanced geometry.
The indirect drawing buffer is updated with atomic counters from compute shader.

The LOD-sorting allows us to instance few but complex meshes when we're close,
and progressively less and less detail per instance. Last LOD is just point sprites.

The number of blocks in play is ~850k.
With my nVidia GTX760, I can get roughly 300-500 FPS up to 1000 FPS depending on the scene complexity after culling.

LOD0: Blender monkey (Suzanne) (diffuse + specular lighting)<br/>
LOD1: Cube (diffuse lighting)<br/>
LOD2: Point sprites<br/>

Video
======
[YouTube](http://www.youtube.com/watch?v=_K2Wx7lW3fY&feature=youtu.be)

## Requirements
A graphics card and driver supporting OpenGL 4.3 and/or higher.

## Building
To compile, you will need a C compiler and assorted toolchain installed.

    make -f Makefile.libretro

This targets [libretro](http://libretro.com) GL interface, so you need [RetroArch](https://github.com/libretro/RetroArch) installed.

## Running
After building, this command should run the program:

    retroarch -L boxes_libretro.so
