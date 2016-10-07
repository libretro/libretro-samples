# rendering
This sample is an extended version of the 'rendering' sample. 

In addition to demonstrating how to render graphics to the software framebuffer, it also
shows you how to use an optional environment callback feature 
(RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER) which can avoid a 
framebuffer copy and render directly to VRAM, thereby increasing performance.

This environment callback does not have to be implemented by the libretro frontend's video 
driver, therefore you should always provide a fallback path in case it's not available.

Currently, RetroArch's Vulkan video driver implements this feature. This sample effectively
shows you how to gain better framebuffer blitting performance with a software-rendered core 
using Vulkan without having to write any Vulkan code.

## Programming language
C

## Building
To compile, you will need a C compiler and assorted toolchain installed.

	make
