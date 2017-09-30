QT       += core gui network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

OTHER_FILES += \
    NOTICE \
    LICENSE

QMAKE_MAC_SDK = macosx10.9

HEADERS += \
    crypto/CryptoCommon.hpp \
    crypto/File.hpp \
    crypto/UserKey.hpp \
    fs/fvfilewatcher.h \
    fs/fvdropbox.h \
    fs/fvfs.h \
    fs/fvfsexceptions.h \
    fvmainwindow.h \
    fvcontrol.hpp \
    fvcontrolworker.hpp

SOURCES += \
    crypto/CryptoCommon.cpp \
    crypto/File.cpp \
    crypto/UserKey.cpp \
    fs/fvfilewatcher.cpp \
    fs/fvdropbox.cpp \
    fs/fvfs.cpp \
    fs/fvfsexceptions.cpp \
    fvmainwindow.cpp \
    main.cpp \
    fvcontrol.cpp \
    fvcontrolworker.cpp


LIBS += -L"$$_PRO_FILE_PWD_/fs/qtdropbox/lib/"

INCLUDEPATH += fs/qtdropbox/ \ 
                fs/

include(fs/qtdropbox/libqtdropbox.pri)


CONFIG += c++11

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += libsodium

FORMS += \
    fvmainwindow.ui
