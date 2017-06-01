#-------------------------------------------------
#
# Project created by QtCreator 2017-03-10T22:34:19
#
#-------------------------------------------------

QT       += core gui
TEMPLATE = app
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gui
Release: DESTDIR = $$PWD/Release
Debug: DESTDIR = $$PWD/Debug


Debug:LIBS += $$PWD/../core/Debug/libcore.a
Debug:LIBS += $$PWD/../libGeographic/Debug/liblibGeographic.a
Debug:LIBS += $$PWD/../libEasyBMP/Debug/liblibEasyBMP.a

Release:LIBS += $$PWD/../core/Release/libcore.a
Release:LIBS += $$PWD/../libGeographic/Release/liblibGeographic.a
Release:LIBS += $$PWD/../libEasyBMP/Release/liblibEasyBMP.a


INCLUDEPATH = $$PWD/../libGeographic/include/

DEFINES += QT_DEPRECATED_WARNINGS


SOURCES += main.cpp\
        dataprocessor.cpp \
        player.cpp

HEADERS  += dataprocessor.h \
        player.h

RESOURCES += \
    images.qrc

QMAKE_CXXFLAGS += -std=c++0x
