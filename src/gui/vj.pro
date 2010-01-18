# Use 'qmake -o Makefile ttedit.pro'

CONFIG += qt
#debug doesn't do shit
#CONFIG += qt debug

QT += opengl

HEADERS += app.h
HEADERS += mainwin.h
HEADERS += glwidget.h

SOURCES += app.cpp
SOURCES += mainwin.cpp
SOURCES += glwidget.cpp

RESOURCES += vj.qrc

OBJECTS_DIR = ../../obj
MOC_DIR = ../../obj
RCC_DIR = ../../obj
UI_DIR = ../../obj
