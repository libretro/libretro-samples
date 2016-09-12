
MKDIR   ?= mkdir
RM      ?= rm
RM_F    ?= $(RM) -f
RM_RF   ?= $(RM) -rf
MKDIR_P ?= $(MKDIR) -p

cc      := $(CC)
cxx     := $(CXX)
soext   := so
objdir  := obj/

ifneq ($(findstring $(config),win32),)
	soext := dll
endif

target  := cruzes_libretro.$(soext)
sources := cruzes.c ttf2c.c
generated := font24.h font16.h font10.h

libs    := -lm
ldirs   :=
lflags  :=

idirs  := -I.
cflags := -Wall -std=c99
fpic   := -fPIC

cflags_debug   := -g
cflags_release := -O2 -g

ifneq ($(findstring debug,$(config)),)
	cflags += $(cflags_debug)
else
	cflags += $(cflags_release)
endif

ifneq ($(findstring sse2,$(config)),)
	cflags += -msse2
endif

ifneq ($(findstring lto,$(config)),)
	cflags += -flto
	lflags += -flto
endif

ifneq ($(findstring ofast,$(config)),)
	cflags += -Ofast
endif

ifneq ($(findstring clang,$(config)),)
	cc  := clang
endif

ifneq ($(sanitizer),)
	cflags += -fsanitize=$(sanitizer)
	lflags += -fsanitize=$(sanitizer)
else
	lflags += -Wl,--no-undefined
endif

objects := $(addprefix $(objdir),$(sources:.c=.o))
deps    := $(objects:.o=.d)

.PHONY: all clean

all: $(target)
clean:
	-$(RM_F) $(target) $(objects) $(deps) ttf2c{,.exe}

$(target): $(generated) $(objects) 
	$(cc) -o $(target) $(ldirs) $(lflags) -shared $(fpic) $(objects) $(libs)

font24.h: ttf2c $(objdir)Carlito-Regular.ttf
	./ttf2c 24 font24 $(objdir)Carlito-Regular.ttf font24.h

font16.h: ttf2c $(objdir)Carlito-Regular.ttf
	./ttf2c 16 font16 $(objdir)Carlito-Regular.ttf font16.h

font10.h: ttf2c $(objdir)Carlito-Regular.ttf
	./ttf2c 10 font10 $(objdir)Carlito-Regular.ttf font10.h

ttf2c: ttf2c.c Makefile
	$(cc) -Wall -g -DTTF2C_MAIN -o $@ $< -lm

$(objdir)%.o: %.c $(generated)
	-@$(MKDIR_P) $(dir $@)
	$(cc) $(idirs) $(cflags) $(fpic) -MMD -c -o $@ $<
	-@sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
		-e '/^$$/ d' -e 's/$$/ :/' -i'' $(objdir)$*.d

-include $(deps)


