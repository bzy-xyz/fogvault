#ifndef FVMAINWINDOW_H
#define FVMAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QFileDialog>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>
#include <QSharedPointer>
#include "crypto/UserKey.hpp"
#include "fs/fvfs.h"
#include "fvcontrol.hpp"

namespace Ui {
class FvMainWindow;
}

class FvMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit FvMainWindow(QWidget *parent = 0);
    ~FvMainWindow();
    
private slots:
    void on_pushButton_clicked();

    void on_genKeyButton_clicked();

    void on_exportKeyButton_clicked();

    void on_loadKeyButton_clicked();

    void on_manageDBButton_clicked();

    void on_exportPubKeyButton_clicked();

private:
    Ui::FvMainWindow *ui;
    QSharedPointer<FVUserKeyPair> keyPair;
    QSharedPointer<FvFs> fs;
    QSharedPointer<FVControl> ctl;
};

#endif // FVMAINWINDOW_H
