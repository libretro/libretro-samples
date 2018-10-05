# midi_test_libretro
A MIDI test core written in C for libretro.

Demonstrates usage of the MIDI API:
- Input/Output (input to output loopback)
- Output (metronome, one octave keyboard)

## Usage
Keyboard keys 0,1,2 and 3 select current test:
0 None, 1 Metronome, 2 Loopback, 3 Keyboard.

## Programming language
C

## Building
To compile, you will need a C compiler and assorted toolchain installed.

	make
