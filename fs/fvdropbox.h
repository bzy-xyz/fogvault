#ifndef FVDROPBOX_H
#define FVDROPBOX_H
#include <QObject>
#include <qdropbox.h>
#include <qfile.h>
#define APP_SECRET "u2vy0nkokq5dyr3"
#define APP_KEY "nbzilukwd51aanm"
#define DROPBOX_NEED_CONFIRMATION -1
#define TOKENFILENAME "FvToken"
class FvDropbox : public QObject
{
    Q_OBJECT
public:
    explicit FvDropbox(QObject *parent = 0);

    //Should be called by the UI. If it returns DROPBOX_NEED_CONFIRMATION, it will have opened a LINK to the
    // dropbox athorization page and will be waiting for the FvDropboxFinishConnecting() to be received from the UI;
    //if it returns true (1), nothing has to be done.
    int FvDropboxTryConnect();

    //Saves the tokens to disk.
    int saveTokenToDisk();


    int uploadFile(const QString & localPath, const QString& remotePath);



    int downloadFile(const QString & remotePath, const QString& localPath);



signals:

public slots:
    //To be called after the user accepts.
    int FvDropboxFinishConnecting();
private:
    QDropbox dropbox;
    QFile fvTokenFile;

};

#endif // FVDROPBOX_H
