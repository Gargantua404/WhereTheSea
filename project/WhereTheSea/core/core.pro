#-------------------------------------------------
#
# Project created by QtCreator 2017-05-03T22:56:43
#
#-------------------------------------------------

QT       -= core gui

TEMPLATE = lib
CONFIG += staticlib
INCLUDEPATH = $$PWD/../libGeographic/include/

TARGET = core
Release: DESTDIR = $$PWD/Release
Debug: DESTDIR = $$PWD/Debug

DEFINES += QT_DEPRECATED_WARNINGS


SOURCES +=\
    Radar.cpp

HEADERS += \
    Radar.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

QMAKE_CXXFLAGS += -std=c++0x
