#include "fvmainwindow.h"
#include "fvcontrol.hpp"
#include "ui_fvmainwindow.h"
#include "crypto/UserKey.hpp"
#include "fs/fvfs.h"
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>

FvMainWindow::FvMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ctl(fs),
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
    // TODO make this open into FogVault folder instead of whatever it's doing now
    QDesktopServices::openUrl(QUrl("file:///"));
}

void FvMainWindow::on_genKeyButton_clicked()
{
    keyPair = QSharedPointer<FVUserKeyPair>(new FVUserKeyPair());
    QLabel* uk_label = this->findChild<QLabel*>("uk_label");
    uk_label->setText("Generated");
}

void FvMainWindow::on_exportKeyButton_clicked()
{
    try {
        QString filename = QFileDialog::getSaveFileName();
        FVUserKeyPair* data = keyPair.data();
        if (filename != "") {
            bool ok;
            QString password = QInputDialog::getText(this, tr("Set Password"),
                                                     tr("Password for key file:"), QLineEdit::Normal,
                                                     "", &ok);
            data->SaveToFile(filename, password);
        }
    }
    catch (FVExceptionBase &e) {
        // TODO inform user that file may be saved improperly
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
            bool ok;
            QString password = QInputDialog::getText(this, tr("Enter Password"),
                                                     tr("Password for key file:"), QLineEdit::Normal,
                                                     "", &ok);
            QSharedPointer<FVUserKeyPair> newKeyPair = QSharedPointer<FVUserKeyPair>(new FVUserKeyPair(filename, password));
            keyPair = newKeyPair;
            QLabel* uk_label = this->findChild<QLabel*>("uk_label");
            uk_label->setText("Loaded");
        }
    }
    catch (FVExceptionBase &e) {
        // TODO Let user know that there was a problem reading the key
    }
    catch (...) {
        // User probably did something unexpected with the UI.
    }
}


void FvMainWindow::on_manageDBButton_clicked()
{
    fs.FvDropboxTryConnect();
    QMessageBox::StandardButton reply;
      reply = QMessageBox::question(this, "DB Connect", "Allow Dropbox?");
      if (reply == QMessageBox::Yes) {
          int connect = fs.FvDropboxFinishConnecting();
          if (connect > 0) {
            QLabel* connect_label = this->findChild<QLabel*>("connect_label");
            connect_label->setText("Yes");
          }
      }
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
        // TODO inform user that file may be saved improperly
    }
    catch (...) {
        // user probably did something unexpected
    }
}
