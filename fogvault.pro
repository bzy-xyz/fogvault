QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

OTHER_FILES += \
    NOTICE \
    LICENSE

QMAKE_MAC_SDK = macosx10.9

HEADERS += \
    crypto/CryptoCommon.hpp \
    crypto/File.hpp \
    crypto/UserKey.hpp

SOURCES += \
    crypto/CryptoCommon.cpp \
    crypto/File.cpp \
    crypto/UserKey.cpp


LIBS += -L"$$_PRO_FILE_PWD_/fs/qtdropbox/lib/"
INCLUDEPATH += fs/qtdropbox/

include(fs/qtdropbox/libqtdropbox.pri)


CONFIG += c++11

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += libsodium
