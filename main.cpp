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

    FvDropbox dbx;
    FvFileWatcher fw(0, QDir::home().absoluteFilePath("FogVault/"));

    FVControl ctl(dbx, fw);

    return a.exec();
}
