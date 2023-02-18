version       = "0.0.1"
author        = "David Konsumer"
description   = "Example libretro core"
license       = "MIT"
srcDir        = "src"

requires "nim >= 1.6.10"

import os

task core, "Build your libretro core":
  selfExec("c --outDir:. --app:lib --gc:none --passL:-fPIC src/example_libretro.nim")

task clean, "Clean built files":
  for file in listFiles("."):
    let ext = splitFile(file).ext
    if ext == ".dll" or ext == ".so" or ext == ".dylib":
      echo "Deleting ", file
      rmFile(file)