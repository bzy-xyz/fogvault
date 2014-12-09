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
    int FvDropboxFinishConnecting();

};

#endif // FVFS_H
