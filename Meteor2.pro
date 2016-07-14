#-------------------------------------------------
#
# Project created by QtCreator 2016-07-01T19:08:09
#
#-------------------------------------------------

QT       += core gui sql xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Meteor2
TEMPLATE = app


SOURCES += main.cpp\
        interface.cpp \
    iolog.cpp \
    storage.cpp

HEADERS  += interface.h \
    iolog.h \
    storage.h


QMAKE_LFLAGS_RELEASE += -static -static-libgcc

DISTFILES +=
