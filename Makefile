#
# Unified Makefile for Virtual Jaguar GCC/SDL Portable Jaguar Emulator
#
# Note that SYSTYPE, EXESUFFIX, GLLIB, ICON, and SDLLIBTYPE are passed in by the
# script called "compile". The unified makefile should make life a little
# easier for the maintainers. :-)
#
# To the Gentoo ebuild maintainers: If something is silly in the compile script
# and/or makefile, why don't you enlighten us poor schmucks as to what's wrong? ;-)
#

CC         = gcc
LD         = gcc
TARGET     = vj$(EXESUFFIX)

# Note that we use optimization level 2 instead of 3--3 doesn't seem to gain much over 2
CFLAGS = -Wall -Wno-switch -Wno-non-virtual-dtor -O2 -D$(SYSTYPE) \
		-ffast-math -fomit-frame-pointer `sdl-config --cflags`
#		-fomit-frame-pointer `sdl-config --cflags` -g
#		-fomit-frame-pointer `sdl-config --cflags` -DLOG_UNMAPPED_MEMORY_ACCESSES

LDFLAGS =

LIBS = -L/usr/local/lib -L/usr/lib `sdl-config $(SDLLIBTYPE)` -lstdc++ -lz $(GLLIB)

INCS = -I. -Isrc -Isrc/include -I/usr/local/include -I/usr/include

THECC = $(CC) $(CFLAGS) $(INCS)

OBJS = \
	obj/blitter.o \
	obj/cdrom.o \
	obj/cdintf.o \
	obj/clock.o \
	obj/crc32.o \
	obj/dac.o \
	obj/dsp.o \
	obj/eeprom.o \
	obj/gpu.o \
	obj/gui.o \
	obj/jagdasm.o \
	obj/jaguar.o \
	obj/jerry.o \
	obj/joystick.o \
	obj/log.o \
	obj/m68kcpu.o obj/m68kops.o obj/m68kopac.o obj/m68kopdm.o obj/m68kopnz.o \
	obj/m68kdasm.o \
	obj/memory.o \
	obj/objectp.o \
	obj/sdlemu_config.o \
	obj/sdlemu_opengl.o \
	obj/settings.o \
	obj/tom.o \
	obj/unzip.o \
	obj/version.o \
	obj/video.o \
	obj/vj.o \
	$(ICON)
#	obj/anajoy.o 
#	obj/cdbios.o 
#	obj/cdi.o 

all: obj $(TARGET)

clean:
	rm -rf obj
	rm -f ./vj$(EXESUFFIX)

obj:
	mkdir obj

obj/%.o: src/%.c src/include/%.h
	$(THECC) -c $< -o $@

obj/%.o: src/%.cpp src/include/%.h
	$(THECC) -c $< -o $@

obj/%.o: src/%.c
	$(THECC) -c $< -o $@

obj/%.o: src/%.cpp
	$(THECC) -c $< -o $@

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
#	strip --strip-all vj$(EXESUFFIX)
#	upx -9 vj$(EXESUFFIX)

# Dependencies

obj/dac.o:       src/include/settings.h
obj/eeprom.o:    src/include/settings.h
obj/gui.o:       src/include/settings.h
obj/jaguar.o:    src/include/settings.h
obj/joystick.o:  src/include/settings.h
obj/tom.o:       src/include/settings.h
obj/video.o:     src/include/settings.h
obj/vj.o:        src/include/settings.h

# Other stuff that has unusual dependencies

obj/gui.o: src/gui.cpp src/include/gui.h src/include/font1.h src/include/font14pt.h src/include/guielements.h
obj/cdintf.o: src/cdintf.cpp src/cdintf_win32.cpp src/cdintf_osx.cpp src/cdintf_linux.cpp src/include/cdintf.h

#
# Musashi specific stuffola
#

obj/m68kcpu.o: obj/m68kops.h src/m68k.h src/m68kconf.h
	$(CC) $(CFLAGS) -Iobj -c src/m68kcpu.c -o obj/m68kcpu.o

obj/m68kops.o: obj/m68kmake$(EXESUFFIX) obj/m68kops.h obj/m68kops.c src/m68k.h src/m68kconf.h
	$(CC) $(CFLAGS) -Isrc -c obj/m68kops.c -o obj/m68kops.o

obj/m68kopac.o: obj/m68kmake$(EXESUFFIX) obj/m68kops.h obj/m68kopac.c src/m68k.h src/m68kconf.h
	$(CC) $(CFLAGS) -Isrc -c obj/m68kopac.c -o obj/m68kopac.o

obj/m68kopdm.o: obj/m68kmake$(EXESUFFIX) obj/m68kops.h obj/m68kopdm.c src/m68k.h src/m68kconf.h
	$(CC) $(CFLAGS) -Isrc -c obj/m68kopdm.c -o obj/m68kopdm.o

obj/m68kopnz.o: obj/m68kmake$(EXESUFFIX) obj/m68kops.h obj/m68kopnz.c src/m68k.h src/m68kconf.h
	$(CC) $(CFLAGS) -Isrc -c obj/m68kopnz.c -o obj/m68kopnz.o

obj/m68kdasm.o: src/m68kdasm.c src/m68k.h src/m68kconf.h
	$(CC) $(CFLAGS) -Isrc -c src/m68kdasm.c -o obj/m68kdasm.o

obj/m68kops.h: obj/m68kmake$(EXESUFFIX)
	obj/m68kmake obj src/m68k_in.c

obj/m68kmake$(EXESUFFIX): src/m68kmake.c src/m68k_in.c
	$(CC) $(WARNINGS) src/m68kmake.c -o obj/m68kmake$(EXESUFFIX)
