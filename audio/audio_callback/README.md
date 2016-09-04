# audio_callback
This sample demonstrates how to generate sound using libretro API with an audio callback.

Audio gets its own callback function to live in and libretro's audio callback function
gets called independently of retro_run iteration in this case.

In case the audio callback cannot be setup (a libretro frontend might not support it), 
we fallback to non-callback based audio instead.

The audio samplerate in this sample is set to 3000KHz, sound is 16-bit stereo.

## Programming language
C

## Building
To compile, you will need a C compiler and assorted toolchain installed.

	make
