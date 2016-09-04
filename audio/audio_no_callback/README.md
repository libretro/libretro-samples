# audio_no_callback
This sample demonstrates how to generate sound using libretro API with no audio callback.

What this means is that libretro's audio callback function has to be called inside retro_run
at least once per run.

The audio samplerate in this sample is set to 3000KHz, sound is 16-bit stereo.

## Programming language
C

## Building
To compile, you will need a C compiler and assorted toolchain installed.

	make
