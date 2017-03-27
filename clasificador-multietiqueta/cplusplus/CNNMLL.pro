QT += core
QT -= gui

CONFIG += c++11
INCLUDEPATH = /usr/include
LIBS = -L/usr/lib/x86_64-linux-gnu -lm -lpthread -lX11

TARGET = CNNMLL
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp
SOURCES += pugixml.cpp

HEADERS += \
    Documento.h
