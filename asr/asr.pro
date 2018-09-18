#-------------------------------------------------
#
# Project created by QtCreator 2018-08-04T15:42:40
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = asr
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    ../common/alsa.cpp \
    ../common/utility.cpp \
    i2coper.cpp \
    parsexml.cpp

OTHER_FILES += \
    ../common/libmsc.so

LIBS += -L$$PWD/../common/ -lmsc -lxml2
INCLUDEPATH += $$PWD/../include/ $$PWD/../common/

HEADERS += \
    alsa.h \
    ../common/utility.h \
    i2coper.h \
    parsexml.h

LIBS += -lasound

target.path=/home/root/xunfei/asr
INSTALLS+=target
