#
# Virtual Jaguar Qt project file
#
# by James Hammons
# Copyright (C) 2011 Underground Software
#
# See the README and GPLv3 files for licensing and warranty information
#
# NOTE: M68000 core is built and linked in as a library, so there should be no
#       more problems with using the qmake build system as-is. :-)
#       Other than on the Mac, where it stupidly defaults to making XCode
#       binaries. >:-( Well, we fixed it in the Makefile, by doing platform
#       detection there. :-/
#

TARGET     = virtualjaguar
CONFIG    += qt warn_on release
# debug
RESOURCES += src/gui/virtualjaguar.qrc
LIBS      += -Lobj -Lsrc/m68000/obj -ljaguarcore -lz -lm68k
QT        += opengl widgets

# We stuff all the intermediate crap into obj/ so it won't confuse us mere
# mortals ;-)
OBJECTS_DIR = obj
MOC_DIR     = obj
RCC_DIR     = obj
UI_DIR      = obj

# Platform specific defines
win32     { DEFINES += __GCCWIN32__ }
else:macx { DEFINES += __GCCUNIX__ __THINK_STUPID__ }
else:unix { DEFINES += __GCCUNIX__ }

# SDL (to link statically on Mac)
macx { LIBS += `sdl-config --static-libs` }
else { LIBS += `$(CROSS)sdl-config --libs` }

# Icon on Win32, Mac
#win32 { LIBS += res/vj-ico.o }
#win32 { ICON = res/vj.ico }
#win32 { LIBS += res/vj.o; $(CROSS)windres -i res/vj.rc -o vj.o --include-dir=./res }
win32 { RC_FILE = res/vj.rc }
macx  { ICON = res/vj-icon.icns }

# C/C++ flags...
# NOTE: May have to put -Wall back in, but only on non-release cycles. It can
#       cause problems if you're not careful. (Can do this via command line in
#       qmake)
QMAKE_CFLAGS += `$(CROSS)sdl-config --cflags`
QMAKE_CXXFLAGS += `$(CROSS)sdl-config --cflags`

# Need to add libcdio stuffola (checking/including)...

# Translations. NB: Nobody has stepped up to do any :-P so these are dummy
# translations
# Removed for now, they interfere with proper running in non-English locales for
# some reason. :-/
#TRANSLATIONS = \
#	virtualjaguar_fr.ts \
#	virtualjaguar_gr.ts

INCLUDEPATH += \
	src \
	src/gui

DEPENDPATH = \
	src \
	src/gui \
	src/gui/debug \
	src/m68000

# The GUI

HEADERS = \
	src/gui/about.h \
	src/gui/alpinetab.h \
	src/gui/app.h \
	src/gui/configdialog.h \
	src/gui/controllertab.h \
	src/gui/controllerwidget.h \
	src/gui/filelistmodel.h \
	src/gui/filepicker.h \
	src/gui/filethread.h \
	src/gui/gamepad.h \
	src/gui/generaltab.h \
	src/gui/glwidget.h \
	src/gui/help.h \
	src/gui/imagedelegate.h \
	src/gui/keygrabber.h \
	src/gui/mainwin.h \
	src/gui/profile.h \
	src/gui/debug/cpubrowser.h \
	src/gui/debug/m68kdasmbrowser.h \
	src/gui/debug/memorybrowser.h \
	src/gui/debug/opbrowser.h \
	src/gui/debug/riscdasmbrowser.h

SOURCES = \
	src/gui/about.cpp \
	src/gui/alpinetab.cpp \
	src/gui/app.cpp \
	src/gui/configdialog.cpp \
	src/gui/controllertab.cpp \
	src/gui/controllerwidget.cpp \
	src/gui/filelistmodel.cpp \
	src/gui/filepicker.cpp \
	src/gui/filethread.cpp \
	src/gui/gamepad.cpp \
	src/gui/generaltab.cpp \
	src/gui/glwidget.cpp \
	src/gui/help.cpp \
	src/gui/imagedelegate.cpp \
	src/gui/keygrabber.cpp \
	src/gui/mainwin.cpp \
	src/gui/profile.cpp \
	src/gui/debug/cpubrowser.cpp \
	src/gui/debug/m68kdasmbrowser.cpp \
	src/gui/debug/memorybrowser.cpp \
	src/gui/debug/opbrowser.cpp \
	src/gui/debug/riscdasmbrowser.cpp

