# -------------------------------------------------
# Project created by QtCreator 2010-09-23T16:43:16
# -------------------------------------------------
QT -= core \
    gui
TARGET = conv2PyrTiff
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp \
    pyrTiffUtil.cpp
HEADERS += pyrTiffUtil.h

INCLUDEPATH += C:/OpenCV2.1/include/opencv ../bigtiff/tiff-4.0/libtiff/

LIBS += ../bigtiff/lib/libtiff.a C:/OpenCV2.1/lib/cv210.lib C:/OpenCV2.1/lib/cxcore210.lib
