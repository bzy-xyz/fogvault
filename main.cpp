#include "fvmainwindow.h"
#include "fs/fvdropbox.h"
#include "fs/fvfilewatcher.h"
#include "crypto/UserKey.hpp"
#include <QApplication>
#include <QDir>
#include "fvcontrol.hpp"

int main(int argc, char *argv[])
{
    FVCryptoInit();

    QApplication a(argc, argv);
    FvMainWindow w;
    w.show();

    return a.exec();
}
