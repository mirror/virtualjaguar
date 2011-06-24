#
# Unified Makefile for Virtual Jaguar GCC/SDL Portable Jaguar Emulator
#
# by James L. Hammons
#
# This software is licensed under the GPL v2 or any later version. See the
# file GPL.TXT for details. ;-)
#

# NOTE: Qt and OpenGL libs are a dependency, but are not checked for.

# Figure out which system we're compiling for, and set the appropriate variables

OSTYPE := $(shell uname -a)

ifeq "$(findstring Msys,$(OSTYPE))" "Msys"			# Win32

SYSTYPE    := __GCCWIN32__
EXESUFFIX  := .exe
GLLIB      := -lopengl32
ICON       := obj/icon.o
SDLLIBTYPE := --libs
MSG        := Win32 on MinGW

else ifeq "$(findstring Darwin,$(OSTYPE))" "Darwin"	# Should catch both 'darwin' and 'darwin7.0'

SYSTYPE    := __GCCUNIX__ -D_OSX_
EXESUFFIX  :=
GLLIB      :=
ICON       :=
SDLLIBTYPE := --static-libs
MSG        := Mac OS X

else ifeq "$(findstring Linux,$(OSTYPE))" "Linux"		# Linux

SYSTYPE    := __GCCUNIX__
EXESUFFIX  :=
GLLIB      := -lGL -lGLU
ICON       :=
SDLLIBTYPE := --libs
MSG        := GNU/Linux

else											# ???

$(error OS TYPE UNDETECTED)

endif

# Set vars for libcdio
ifneq "$(shell pkg-config --silence-errors --libs libcdio)" ""
HAVECDIO := -DHAVE_LIB_CDIO
CDIOLIB  := -lcdio
else
HAVECDIO :=
CDIOLIB  :=
endif

CC         := gcc
LD         := gcc
TARGET     := virtualjaguar

# Note that we use optimization level 2 instead of 3--3 doesn't seem to gain much over 2
CFLAGS   := -MMD -Wall -Wno-switch -O2 -D$(SYSTYPE) -ffast-math -fomit-frame-pointer `sdl-config --cflags`
CPPFLAGS := -MMD -Wall -Wno-switch -Wno-non-virtual-dtor -O2 -D$(SYSTYPE) -Wno-trigraphs \
		$(HAVECDIO) -ffast-math -fomit-frame-pointer `sdl-config --cflags` -g
#		-fomit-frame-pointer `sdl-config --cflags` -g
#		-fomit-frame-pointer `sdl-config --cflags` -DLOG_UNMAPPED_MEMORY_ACCESSES

LDFLAGS :=

LIBS := -L/usr/local/lib -L/usr/lib `sdl-config $(SDLLIBTYPE)` -lstdc++ -lz $(GLLIB) $(CDIOLIB) `pkg-config --libs QtGui QtOpenGL`

INCS := -I. -I./src -I./src/gui -I/usr/local/include -I/usr/include

OBJS := \
	obj/m68kcpu.o       \
	obj/m68kops.o       \
	obj/m68kopac.o      \
	obj/m68kopdm.o      \
	obj/m68kopnz.o      \
	obj/m68kdasm.o      \
\
	obj/about.o             \
	obj/app.o               \
	obj/configdialog.o      \
	obj/moc_configdialog.o  \
	obj/generaltab.o        \
	obj/moc_generaltab.o    \
	obj/controllertab.o     \
	obj/moc_controllertab.o \
	obj/filepicker.o        \
	obj/moc_filepicker.o    \
	obj/filelistmodel.o     \
	obj/filethread.o        \
	obj/imagedelegate.o     \
	obj/moc_filethread.o    \
	obj/mainwin.o           \
	obj/moc_mainwin.o       \
	obj/glwidget.o          \
	obj/moc_glwidget.o      \
	obj/qrc_virtualjaguar.o \
\
	obj/blitter.o       \
	obj/cdrom.o         \
	obj/cdintf.o        \
	obj/crc32.o         \
	obj/dac.o           \
	obj/dsp.o           \
	obj/eeprom.o        \
	obj/event.o         \
	obj/file.o          \
	obj/filedb.o        \
	obj/gpu.o           \
	obj/jagdasm.o       \
	obj/jaguar.o        \
	obj/jerry.o         \
	obj/joystick.o      \
	obj/log.o           \
	obj/memory.o        \
	obj/mmu.o           \
	obj/objectp.o       \
	obj/sdlemu_config.o \
	obj/sdlemu_opengl.o \
	obj/settings.o      \
	obj/state.o         \
	obj/tom.o           \
	obj/unzip.o         \
	obj/video.o         \
	obj/virtualjaguar.o \
	obj/wavetable.o     \
	$(ICON)

# Targets for convenience sake, not "real" targets
.PHONY: clean

all: checkenv message obj $(TARGET)$(EXESUFFIX)
	@echo
	@echo -e "\033[01;33m***\033[00;32m Looks like it compiled OK... Give it a whirl!\033[00m"
	@echo

# Check the compilation environment, barf if not appropriate

checkenv: msg-check-env check-sdl check-zlib check-cdio check-gl check-qt ;

#check-sdl: msg-ck-sdl $(if $(strip ),,msg-no-sdl stop-on-error)
check-sdl: msg-ck-sdl $(if $(shell which sdl-config),,msg-no-sdl stop-on-error)
	@echo -e "\033[01;37mOK\033[00m"

msg-ck-sdl:
	@echo -en "   \033[00;32mSDL... \033[00m"

msg-no-sdl:
	@echo -e "\033[01;37mNOT FOUND\033[00m"
	@echo
	@echo -e "\033[01;33mIt seems that you don't have the SDL development libraries installed. If you"
	@echo -e "have installed them, make sure that the sdl-config file is somewhere in your"
	@echo -e "path and is executable.\033[00m"
	@echo

check-zlib: msg-ck-zlib $(if $(shell pkg-config --silence-errors --libs zlib),,msg-no-zlib stop-on-error)
	@echo -e "\033[01;37mOK\033[00m"

msg-ck-zlib:
	@echo -en "   \033[00;32mZLIB... \033[00m"

msg-no-zlib:
	@echo -e "\033[01;37mNOT FOUND\033[00m"
	@echo
	@echo -e "\033[01;33mIt seems that you don't have ZLIB installed. If you have installed it, make"
	@echo -e "sure that the pkg-config file is somewhere in your path and is executable.\033[00m"
	@echo

#NOTE that this check shouldn't be fatal, we can bounce back from it by excluding CD support
check-cdio: msg-ck-cdio $(if $(CDIOLIB),msg-cdio,msg-no-cdio) ;

msg-ck-cdio:
	@echo -en "   \033[00;32mLIBCDIO... \033[00m"

msg-cdio:
	@echo -e "\033[01;37mOK\033[00m"

msg-no-cdio:
	@echo -e "\033[01;37mNOT FOUND\033[00m"
	@echo
	@echo -e "\033[01;33mIt seems that you don't have LIBCDIO installed. Since this is not fatal,"
	@echo -e "Virtual Jaguar will be built WITHOUT CD support.\033[00m"
	@echo

check-gl: msg-ck-gl
	@echo -e "*** GL CHECK NOT IMPLEMENTED ***"

msg-ck-gl:
	@echo -en "   \033[00;32mOpenGL... \033[00m"

#NOTE: Can check for Qt by using 'qmake -v'
check-qt: msg-ck-qt
	@echo -e "*** QT CHECK NOT IMPLEMENTED ***"

msg-ck-qt:
	@echo -en "   \033[00;32mQt... \033[00m"

stop-on-error: ; $(error COMPILATION ENVIRONMENT)

msg-check-env:
	@echo
	@echo -e "\033[01;33m***\033[00;32m Checking compilation environment: \033[00m"
	@echo

message:
	@echo
	@echo -e "\033[01;33m***\033[00;32m Building Virtual Jaguar for $(MSG)...\033[00m"
	@echo

clean:
	@echo
	@echo -en "\033[01;33m***\033[00;32m Cleaning out the garbage...\033[00m"
	@rm -rf obj
	@rm -f ./$(TARGET)$(EXESUFFIX)
	@echo -e "\033[01;37mdone!\033[00m"

obj:
	@mkdir obj

# This is only done for Win32 at the moment...

ifneq "" "$(ICON)"
$(ICON): res/$(TARGET).rc res/$(TARGET).ico
	@echo -e "\033[01;33m***\033[00;32m Processing icon...\033[00m"
	@windres -i res/$(TARGET).rc -o $(ICON) --include-dir=./res
endif

# Main source compilation (implicit rules)...

obj/%.o: src/%.c
	@echo -e "\033[01;33m***\033[00;32m Compiling $<...\033[00m"
	@$(CC) $(CFLAGS) $(INCS) -c $< -o $@

obj/%.o: src/%.cpp
	@echo -e "\033[01;33m***\033[00;32m Compiling $<...\033[00m"
	@$(CC) $(CPPFLAGS) $(INCS) -c $< -o $@

$(TARGET)$(EXESUFFIX): $(OBJS)
	@echo -e "\033[01;33m***\033[00;32m Linking it all together...\033[00m"
	@$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
#	strip --strip-all vj$(EXESUFFIX)
#	upx -9 vj$(EXESUFFIX)

# Qt GUI stuff...

src/gui/Makefile: src/gui/vj-gui.pro src/gui/vj-gui.qrc
	@echo -e "\033[01;33m***\033[00;32m Creating Qt Makefile...\033[00m"
	@qmake -o src/gui/Makefile src/gui/vj-gui.pro

#obj/%.o: src/gui/%.cpp src/gui/Makefile
#	@echo -e "\033[01;33m***\033[00;32m Compiling $<...\033[00m"
#	@make -C src/gui
#OR:
#	@cd src/gui && make

#QT_SOURCES       = app.cpp \
#		mainwin.cpp ../../obj/moc_mainwin.cpp \
#		../../obj/qrc_vj-gui.cpp
#QT_OBJECTS       = ../../obj/app.o \
#		../../obj/mainwin.o \
#		../../obj/moc_mainwin.o \
#		../../obj/qrc_vj-gui.o
QT_CFLAGS = -MMD `pkg-config --cflags QtGui`
# !!! THESE DEFINES ARE PLATFORM DEPENDENT !!!
QT_DEFINES = -DQT_NO_DEBUG -DQT_OPENGL_LIB -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED
QT_INCPATH = -I/usr/share/qt4/mkspecs/linux-g++ -I./src -I/usr/include/qt4/QtCore -I/usr/include/qt4/QtGui -I/usr/include/qt4/QtOpenGL -I/usr/include/qt4 -I/usr/X11R6/include -I./obj `sdl-config --cflags`
QT_CXXFLAGS = -MMD -pipe -O2 -pipe -D_REENTRANT -Wall -W $(QT_DEFINES) -D$(SYSTYPE) -g

obj/moc_%.cpp: src/gui/%.h
	@echo -e "\033[01;33m***\033[00;32m Creating $@...\033[00m"
	@/usr/bin/moc $(QT_DEFINES) $(QT_INCPATH) $< -o $@

obj/moc_%.o: obj/moc_%.cpp
	@echo -e "\033[01;33m***\033[00;32m Compiling $<...\033[00m"
	@$(CC) -c $(QT_CXXFLAGS) $(QT_INCPATH) -o $@ $<

obj/qrc_virtualjaguar.cpp: src/gui/virtualjaguar.qrc res/vj.xpm
	@echo -e "\033[01;33m***\033[00;32m Creating qrc_virtualjaguar.cpp...\033[00m"
	@/usr/bin/rcc -name virtualjaguar src/gui/virtualjaguar.qrc -o obj/qrc_virtualjaguar.cpp

obj/qrc_%.o: obj/qrc_%.cpp
	@echo -e "\033[01;33m***\033[00;32m Compiling $<...\033[00m"
	@$(CC) -c $(QT_CXXFLAGS) $(QT_INCPATH) -o $@ $<

obj/%.o: src/gui/%.cpp src/gui/%.h
	@echo -e "\033[01;33m***\033[00;32m Compiling $<...\033[00m"
	@$(CC) -c $(QT_CXXFLAGS) $(QT_INCPATH) -o $@ $<

#
# Musashi specific stuffola
#

obj/m68kmake$(EXESUFFIX): src/m68kmake.c src/m68k_in.c
	@echo -e "\033[01;33m***\033[00;32m Preparing to make the Musashi core...\033[00m"
	@$(CC) $(WARNINGS) src/m68kmake.c -o obj/m68kmake$(EXESUFFIX)

obj/m68kops.h obj/m68kops.c obj/m68kopac.c obj/m68kopdm.c obj/m68kopnz.c: obj/m68kmake$(EXESUFFIX)
	@echo -e "\033[01;33m***\033[00;32m Creating m68kops.h...\033[00m"
	@obj/m68kmake obj src/m68k_in.c

obj/m68kcpu.o: obj/m68kops.h src/m68k.h src/m68kconf.h
	@echo -e "\033[01;33m***\033[00;32m Compiling m68kcpu.c...\033[00m"
	@$(CC) $(CFLAGS) -Iobj -c src/m68kcpu.c -o obj/m68kcpu.o

obj/m68kop%.o: obj/m68kop%.c
	@echo -e "\033[01;33m***\033[00;32m Compiling $<...\033[00m"
	@$(CC) $(CFLAGS) -Isrc -c $< -o $@

# Pull in dependencies autogenerated by gcc's -MMD switch
# The "-" in front is there just in case they haven't been created yet

-include obj/*.d
