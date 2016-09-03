
ifeq ($(platform),)
platform = unix
ifeq ($(shell uname -a),)
   platform = win
else ifneq ($(findstring MINGW,$(shell uname -a)),)
   platform = win
else ifneq ($(findstring Darwin,$(shell uname -a)),)
   platform = osx
else ifneq ($(findstring win,$(shell uname -a)),)
   platform = win
endif
endif

TARGET_NAME := boxes

ifeq ($(platform), unix)
   TARGET := $(TARGET_NAME)_libretro.so
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=link.T -Wl,--no-undefined
   GL_LIB := -lGL
   INCFLAGS += -I. -Igl
else ifeq ($(platform), osx)
   TARGET := $(TARGET_NAME)_libretro.dylib
   fpic := -fPIC
   SHARED := -dynamiclib
   GL_LIB := -framework OpenGL
   INCFLAGS += -I. -Igl
else
   TARGET := $(TARGET_NAME)_libretro.dll
   SHARED := -shared -static-libgcc -static-libstdc++ -s -Wl,--version-script=link.T -Wl,--no-undefined
   GL_LIB := -lopengl32
   INCFLAGS += -I. -Igl -IGLcore
endif

CXXFLAGS += $(INCFLAGS)

ifeq ($(DEBUG), 1)
   CXXFLAGS += -O0 -g -DGL_DEBUG
   CFLAGS += -O0 -g
else
   CXXFLAGS += -O3 -g
   CFLAGS += -O3 -g
endif

CXXFLAGS += -std=gnu++11 -Wall $(fpic) -DHAVE_ZIP_DEFLATE
CFLAGS += -std=gnu99 -Wall $(fpic) -DHAVE_ZIP_DEFLATE

SOURCES := $(wildcard libretro/*.cpp) $(wildcard gl/*.cpp) app/$(TARGET_NAME).cpp
CSOURCES = $(wildcard rpng/*.c) glsym/rglgen.c
LIBS += $(GL_LIB) 
CSOURCES += glsym/glsym_gl.c

OBJECTS := $(SOURCES:.cpp=.o) $(CSOURCES:.c=.o)

all: $(TARGET)

HEADERS := $(wildcard *.hpp) $(wildcard *.h) $(wildcard */*.hpp) $(wildcard */*.h)

$(TARGET): $(OBJECTS)
	$(CXX) $(fpic) $(SHARED) $(INCLUDES) -o $@ $(OBJECTS) $(LIBS) -lm -lz

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: clean

