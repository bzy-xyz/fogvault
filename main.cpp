#include "fvmainwindow.h"
#include "crypto/UserKey.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FVCryptoInit();
    FvMainWindow w;
    w.show();

    return a.exec();
}
