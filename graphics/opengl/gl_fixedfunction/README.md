# libretro_test_gl_fixedfunction
This sample demonstrates a libretro core using fixed-function OpenGL (GL 1.4 and later / OpenGL ES 1.x). It should work on both desktop (OpenGL 1.4 and later) and mobile (OpenGL ES 1.x and later)

## Requirements
On the desktop - A graphics card driver supporting OpenGL 1.4 and/or higher.

On mobile      - A graphics card driver supporting OpenGLES 1.x and/or higher.

## Programming language
C

## Building
To compile, you will need a C compiler and assorted toolchain installed.

    make

This targets [libretro](http://libretro.com) GL interface, so you need a libretro frontend supporting this interface, such as [RetroArch](https://github.com/libretro/RetroArch), installed.

## Running
After building, this command should run the program:

    retroarch -L testgl_ff_libretro.so
