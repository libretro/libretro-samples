version       = "0.0.1"
author        = "David Konsumer"
description   = "Example libretro core"
license       = "MIT"
srcDir        = "src"
bin           = @["example_libretro"]

requires "nim >= 1.6.10"

import os

task core, "Build libretro core":
  selfExec("c -d:release --app:lib --noMain --gc:orc --outDir=. src/example_libretro.nim")

task clean, "Clean built files":
  for file in listFiles("."):
    let ext = splitFile(file).ext
    if ext == ".dll" or ext == ".so" or ext == ".dylib":
      echo "Deleting ", file
      rmFile(file)