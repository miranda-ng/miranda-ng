#-------------------------------------------------
#
# Project created by QtCreator 2013-03-11T15:57:57
#
#-------------------------------------------------

TARGET = qt_clist
TEMPLATE = lib


QT += core gui widgets

DEFINES += QT_CLIST_LIBRARY USING_PCH

SOURCES += src/stdafx.cpp \
    qt_clist.cpp \
    src/init.cpp \
    src/cluiopts.cpp \
    src/clistopts.cpp \
    src/clistmenus.cpp \
    src/clcpaint.cpp \
    src/clcopts.cpp \
    src/clcfonts.cpp

PRECOMPILED_HEADER = src/commonheaders.h

HEADERS += qt_clist.h\
        qt_clist_global.h \
    src/commonheaders.h \
    src/version.h \
    src/resource.h \
    src/clc.h


INCLUDEPATH += ../../include \
            ./

LIBS += ../../bin10/lib/mir_core.lib \
        comctl32.lib \
        comdlg32.lib

RC_FILE = res/resource.rc

