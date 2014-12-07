#-------------------------------------------------
#
# Project created by QtCreator 2014-12-03T18:51:04
#
#-------------------------------------------------

QT       += testlib core gui network xml

TARGET = tst_fstest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app
INCLUDEPATH += ../../ \
             ../../qtdropbox/


SOURCES += tst_fstest.cpp \
    ../../fvdropbox.cpp \
    ../../fvfilewatcher.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    ../../fvdropbox.h \
    ../../fvfilewatcher.h \
    ../../qtdropbox/qdropbox.h \
    ../../qtdropbox/qdropboxaccount.h \
    ../../qtdropbox/qdropboxfile.h \
    ../../qtdropbox/qdropboxfileinfo.h \
    ../../qtdropbox/qdropboxjson.h \
    ../../qtdropbox/qtdropbox.h \
    ../../qtdropbox/qtdropbox_global.h

LIBS += -L"$$_PRO_FILE_PWD_/../../qtdropbox/lib/"
include(../../qtdropbox/libqtdropbox.pri)
