This is an example libretro core, written in nim. This is a translation from [a basic software-rendering example](https://github.com/libretro/libretro-samples/tree/master/video/software/rendering) but you should be able to roughly translate any of the C examples into nim.

Variable/type names are mostly the same as with a C core, with one exception:

- Because nim disregards case in var-names, I had to rename keys like `RETROK_a` to `RETROK_LOWER_A`.

## Usage

Put your core-code in `src/example_libretro.nim` and you can run these:

```
# build the libretro core
nimble core
```

You can run your core like this:

```
# mac
/Applications/RetroArch.app/Contents/MacOS/RetroArch -L libexample_libretro.dylib

# linux
retroarch -L libexample_libretro.so

# windows
retroarch -L libexample_libretro.dll
```
