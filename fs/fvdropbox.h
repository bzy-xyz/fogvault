#ifndef FVDROPBOX_H
#define FVDROPBOX_H
#include <QObject>
#include <qdropbox.h>
#include <qdropboxfile.h>
#include <qfile.h>
#include <fvfsexceptions.h>

//#define APP_SECRET "u2vy0nkokq5dyr3"
//#define APP_KEY "nbzilukwd51aanm"
#define APP_SECRET "173sttsex8y7ggc"
#define APP_KEY "pe03snfwfj8jaqi"
#define DROPBOX_NEED_CONFIRMATION -1
#define TOKENFILENAME "FvToken"
#define DROPBOX_PATH_PREFIX "/sandbox/"
#define DROPBOX_PATH_PREFIX_LENGTH 9
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


    int uploadFile(QFile & localFile, const QString& remotePath, bool overWrite=true);



    int downloadFile(const QString & remotePath, QFile & localFile);

    static QString getAbsoluteRemotePath(const QString& relativePath);

    static QString getRelativeRemotePath(const QString& absolutePath);

signals:

public slots:
    //To be called after the user accepts.
    int FvDropboxFinishConnecting();
private:
    QDropbox dropbox;
    QFile fvTokenFile;

};

#endif // FVDROPBOX_H
