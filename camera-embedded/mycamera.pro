#-------------------------------------------------
#
# Project created by QtCreator 2015-09-19T18:00:58
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mycamera
TEMPLATE = app

DEFINES += QT_NETWORK_LIB

SOURCES += main.cpp \
    videodevice.cpp \
    processImage.cpp

HEADERS  += \
    videodevice.h \
    processImage.h

FORMS += \
    dialog.ui

