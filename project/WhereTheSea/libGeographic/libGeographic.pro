#-------------------------------------------------
#
# Project created by QtCreator 2017-06-01T12:39:15
#
#-------------------------------------------------

QT       -= core gui

TEMPLATE = lib
CONFIG += staticlib
INCLUDEPATH = $$PWD/include/

TARGET = libGeographic
Release: DESTDIR = $$PWD/Release
Debug: DESTDIR = $$PWD/Debug


DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    src/Geocentric.cpp \
    src/LocalCartesian.cpp

HEADERS += \
    include/GeographicLib/Config.h \
    include/GeographicLib/Constants.hpp \
    include/GeographicLib/Geocentric.hpp \
    include/GeographicLib/LocalCartesian.hpp
unix {
    target.path = /usr/lib
    INSTALLS += target
}

QMAKE_CXXFLAGS += -std=c++0x
