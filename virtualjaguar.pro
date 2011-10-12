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
#       Well, we fixed it in the Makefile, by doing platfrom detection there. :-/
#

TARGET     = virtualjaguar
CONFIG    += qt warn_on release debug
RESOURCES += virtualjaguar.qrc
LIBS      += -lz -Lobj -ljaguarcore -lmusashi
QT        += opengl

# We stuff all the intermediate crap into obj/ so it won't confuse us mere mortals ;-)
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
else { LIBS += `sdl-config --libs` }

# C/C++ flags...
# NOTE: May have to put -Wall back in, but only on non-release cycles. It can
#       cause problems if you're not careful. (Can do this via command line in qmake)
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
	src/gui/alpinetab.h \
	src/gui/app.h \
	src/gui/configdialog.h \
	src/gui/controllertab.h \
	src/gui/controllerwidget.h \
	src/gui/filelistmodel.h \
	src/gui/filepicker.h \
	src/gui/filethread.h \
	src/gui/generaltab.h \
	src/gui/glwidget.h \
	src/gui/help.h \
	src/gui/imagedelegate.h \
	src/gui/keygrabber.h \
	src/gui/mainwin.h

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
	src/gui/generaltab.cpp \
	src/gui/glwidget.cpp \
	src/gui/help.cpp \
	src/gui/imagedelegate.cpp \
	src/gui/keygrabber.cpp \
	src/gui/mainwin.cpp
