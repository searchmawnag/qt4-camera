#-------------------------------------------------
#
# Project created by QtCreator 2015-09-22T10:57:29
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = netcamera
TEMPLATE = app

DEFINES += QT_NETWORK_LIB

SOURCES += main.cpp\
        cameradialog.cpp

HEADERS  += cameradialog.h

FORMS    += cameradialog.ui
