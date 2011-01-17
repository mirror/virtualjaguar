# Use 'qmake -o Makefile ttedit.pro'

CONFIG += qt
#debug doesn't do shit
#CONFIG += qt debug

QT += opengl

HEADERS += app.h
HEADERS += mainwin.h
HEADERS += glwidget.h
HEADERS += about.h
HEADERS += filepicker.h
HEADERS += filethread.h
HEADERS += configwin.h

SOURCES += app.cpp
SOURCES += mainwin.cpp
SOURCES += glwidget.cpp
SOURCES += about.cpp
SOURCES += filepicker.cpp
SOURCES += filethread.cpp
SOURCES += configwin.cpp

RESOURCES += virtualjaguar.qrc

OBJECTS_DIR = ../../obj
MOC_DIR = ../../obj
RCC_DIR = ../../obj
UI_DIR = ../../obj
#Nope, don't do dookey
#INCLUDES_DIR = ..
