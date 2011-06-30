#
# Virtual Jaguar Qt project file
#
# by James L. Hammons
# Copyright (C) 2011 Underground Software
#
# See the README and GPLv3 files for licensing and warranty information
#
# NOTE: Musashi is built and linked in as a library, so there should be no more
#       problems with using the qmake build system as-is. :-)
#       Other than on the Mac, where it stupidly defaults to making XCode binaries. >:-(
#

TARGET     = virtualjaguar
CONFIG    += qt warn_on release debug
RESOURCES += virtualjaguar.qrc
#LIBS      += `sdl-config --libs` -lz -Lobj -lmusashi
LIBS      += -lz -Lobj -lmusashi
QT        += opengl

# We stuff all the intermediate crap into obj/ so it won't confuse us mere mortals ;-)
OBJECTS_DIR = obj
MOC_DIR     = obj
RCC_DIR     = obj
UI_DIR      = obj

# Platform specific defines
win32 { DEFINES += __GCCWIN32__ }
macx  { DEFINES += __GCCUNIX__ _OSX_ }
unix  { DEFINES += __GCCUNIX__ }

# SDL
macx       { LIBS += `sdl-config --static-libs` }
win32|unix { LIBS += `sdl-config --libs` }

#CFLAGS   := -MMD -Wall -Wno-switch -O2 -D$(SYSTYPE) -ffast-math -fomit-frame-pointer `sdl-config --cflags`
#CPPFLAGS := -MMD -Wall -Wno-switch -Wno-non-virtual-dtor -O2 -D$(SYSTYPE) -Wno-trigraphs \
#		$(HAVECDIO) -ffast-math -fomit-frame-pointer `sdl-config --cflags` -g

# C/C++ flags...
# NOTE: May have to put -Wall back in, but only on non-release cycles. It can
#       cause problems if you're not careful.
QMAKE_CFLAGS += `sdl-config --cflags` -O2 -ffast-math -fomit-frame-pointer
QMAKE_CXXFLAGS += `sdl-config --cflags` -O2 -ffast-math -fomit-frame-pointer

# Need to add libcdio stuffola (checking/including)...

INCLUDEPATH += \
	src \
	src/gui

DEPENDPATH = \
	src \
	src/gui

# The GUI

HEADERS = \
	src/gui/about.h \
	src/gui/app.h \
	src/gui/configdialog.h \
	src/gui/controllertab.h \
	src/gui/filelistmodel.h \
	src/gui/filepicker.h \
	src/gui/filethread.h \
	src/gui/generaltab.h \
	src/gui/glwidget.h \
	src/gui/imagedelegate.h \
	src/gui/mainwin.h

SOURCES = \
	src/gui/about.cpp \
	src/gui/app.cpp \
	src/gui/configdialog.cpp \
	src/gui/controllertab.cpp \
	src/gui/filelistmodel.cpp \
	src/gui/filepicker.cpp \
	src/gui/filethread.cpp \
	src/gui/generaltab.cpp \
	src/gui/glwidget.cpp \
	src/gui/imagedelegate.cpp \
	src/gui/mainwin.cpp

# The core (soon to be made into a library of its own)

HEADERS += \
	src/blitter.h       \
	src/cdintf.h        \
	src/cdrom.h         \
	src/crc32.h         \
	src/dac.h           \
	src/dsp.h           \
	src/eeprom.h        \
	src/event.h         \
	src/file.h          \
	src/filedb.h        \
	src/gpu.h           \
	src/jagdasm.h       \
	src/jaguar.h        \
	src/jerry.h         \
	src/joystick.h      \
	src/log.h           \
	src/memory.h        \
	src/mmu.h           \
	src/objectp.h       \
	src/settings.h      \
	src/state.h         \
	src/tom.h           \
	src/unzip.h         \
	src/universalhdr.h  \
	src/wavetable.h

SOURCES += \
	src/blitter.cpp       \
	src/cdintf.cpp        \
	src/cdrom.cpp         \
	src/crc32.cpp         \
	src/dac.cpp           \
	src/dsp.cpp           \
	src/eeprom.cpp        \
	src/event.cpp         \
	src/file.cpp          \
	src/filedb.cpp        \
	src/gpu.cpp           \
	src/jagdasm.cpp       \
	src/jaguar.cpp        \
	src/jerry.cpp         \
	src/joystick.cpp      \
	src/log.cpp           \
	src/memory.cpp        \
	src/mmu.cpp           \
	src/objectp.cpp       \
	src/settings.cpp      \
	src/state.cpp         \
	src/tom.cpp           \
	src/universalhdr.cpp  \
	src/unzip.c           \
	src/wavetable.cpp
