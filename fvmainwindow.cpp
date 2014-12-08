#include "fvmainwindow.h"
#include "ui_fvmainwindow.h"
#include "crypto/UserKey.hpp"
#include <QDebug>

FvMainWindow::FvMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FvMainWindow)
{
    ui->setupUi(this);
}

FvMainWindow::~FvMainWindow()
{
    delete ui;
}

void FvMainWindow::on_pushButton_clicked()
{
    QString path = QDir::toNativeSeparators(QApplication::applicationDirPath());
    QDesktopServices::openUrl(QUrl("file:///" + path));
}

void FvMainWindow::on_genKeyButton_clicked()
{
    keyPair = QSharedPointer<FVUserKeyPair>(new FVUserKeyPair());
}

void FvMainWindow::on_exportKeyButton_clicked()
{
    try {
        QString filename = QFileDialog::getSaveFileName();
        FVUserKeyPair* data = keyPair.data();
        if (filename != "") {
            data->SaveToFile(filename, "password");
        }
    }
    catch (FVExceptionBase &e) {
        // inform user that file may be saved improperly
    }
    catch (...) {
        // user probably did something unexpected
    }
}

void FvMainWindow::on_loadKeyButton_clicked()
{
    try {
        QString filename = QFileDialog::getOpenFileName();
        if (filename != "") {
            QSharedPointer<FVUserKeyPair> newKeyPair = QSharedPointer<FVUserKeyPair>(new FVUserKeyPair(filename, "password"));
            keyPair = newKeyPair;
        }
    }
    catch (FVExceptionBase &e) {
        // Let user know that there was a problem reading the key
    }
    catch (...) {
        // User probably did something unexpected with the UI.
    }
}


void FvMainWindow::on_manageDBButton_clicked()
{
    QDesktopServices::openUrl(QUrl("http://dropbox.com"));
}

void FvMainWindow::on_exportPubKeyButton_clicked()
{
    try {
        QString filename = QFileDialog::getSaveFileName();
        FVUserKeyPair* data = keyPair.data();
        QSharedPointer<FVUserPublicKey> pubKey = data->GetPubKey();
        if (filename != "") {
            pubKey.data()->SaveToFile(filename);
        }
    }
    catch (FVExceptionBase &e) {
        // inform user that file may be saved improperly
    }
    catch (...) {
        // user probably did something unexpected
    }
}
