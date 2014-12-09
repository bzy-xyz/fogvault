#ifndef FVFS_H
#define FVFS_H

#include <QObject>
#include "fvdropbox.h"
#include "fvfilewatcher.h"

class FvFs : public QObject
{
    Q_OBJECT
private:
    FvDropbox fvDropbox;
    FvFileWatcher fvFileWatcher;

public:
    explicit FvFs(QObject *parent = 0);
    FvFs(QString homePath, QObject *parent = 0);

    ///
    /// \brief FvDropboxTryConnect, Should be called to connect to dropbox. Oepns an URL if the person has
    /// no token
    /// \return Returns DROPBOX_NEED_CONFIRMATION if we need the function FvDropboxFinishConnecting();
    /// to be called to confirm that the user acessed dropbox
    /// Returns 1 if it manages to connect and save the token to disk. Returns 0 if it didn't manage to save the token to disk
    ///
    int FvDropboxTryConnect();


    int uploadFile(QFile & localFile, const QString& remotePath, bool overWrite=true);
    int uploadFile(const QString& localPath, bool overWrite=true);

    ///Downloads a file to localFile
    int downloadFile(const QString & remotePath, QFile & localFile);
    ///Downloads a file to the fogvaultHomeDirectory
    int downloadFile(const QString & remotePath);
    ///Downloads a file to the specifiedDir
    int downloadFile(const QString & remotePath, QDir & dir);

    ///TODO
    int compareMapsAndApply(QMap <QString, QDateTime>& timeMap1, QMap <QString, QDateTime> &timeMap2, int function(QString &));
signals:

public slots:
    ///
    /// \brief FvDropboxFinishConnecting to be called after FvDropboxTryConnect() after the user accepted to connect the app to DB.
    /// \return  Returns 1 if it manages to connect and save the token to disk. Returns 0 if it didn't manage to save the token to disk
    ///
    int FvDropboxFinishConnecting();

};

#endif // FVFS_H
