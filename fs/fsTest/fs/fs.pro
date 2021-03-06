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
    ../../fvfilewatcher.cpp \
    ../../fvfs.cpp \
    ../../../crypto/CryptoCommon.cpp \
    ../../../crypto/File.cpp \
    ../../../crypto/UserKey.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    ../../fvdropbox.h \
    ../../qtdropbox/qdropbox.h \
    ../../qtdropbox/qdropboxaccount.h \
    ../../qtdropbox/qdropboxfile.h \
    ../../qtdropbox/qdropboxfileinfo.h \
    ../../qtdropbox/qdropboxjson.h \
    ../../qtdropbox/qtdropbox.h \
    ../../qtdropbox/qtdropbox_global.h \
    ../../qtdropbox/qdropboxdeltaresponse.h \
    ../../fvfilewatcher.h \
    ../../fvfsexceptions.h \
    ../../fvfs.h \
    ../../../crypto/CryptoCommon.hpp \
    ../../../crypto/File.hpp \
    ../../../crypto/UserKey.hpp

LIBS += -L"$$_PRO_FILE_PWD_/../../qtdropbox/lib/"
include(../../qtdropbox/libqtdropbox.pri)
CONFIG += c++11

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += libsodium
