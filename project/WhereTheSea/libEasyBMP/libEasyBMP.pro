#-------------------------------------------------
#
# Project created by QtCreator 2017-05-04T20:20:46
#
#-------------------------------------------------

QT       -= core gui

TEMPLATE = lib
CONFIG += staticlib

TARGET = libEasyBMP
Release: DESTDIR = $$PWD/Release
Debug: DESTDIR = $$PWD/Debug

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += EasyBMP.cpp\

HEADERS += EasyBMP.h\
           EasyBMP_BMP.h\
           EasyBMP_DataStructures.h\
           EasyBMP_VariousBMPutilities.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

QMAKE_CXXFLAGS += -std=c++0x
