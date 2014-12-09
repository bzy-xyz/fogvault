#include "fvfs.h"

FvFs::FvFs(QObject *parent) :
    QObject(parent), fvDropbox()
{

}

FvFs::FvFs(QString homePath, QObject *parent) :
    QObject(parent), fvDropbox(), fvFileWatcher(0,homePath)
{

}

int FvFs::FvDropboxTryConnect(){
    return fvDropbox.FvDropboxTryConnect();
}

int FvFs::FvDropboxFinishConnecting(){
    return fvDropbox.FvDropboxFinishConnecting();
}

int FvFs::uploadFile(QFile & localFile, const QString& remotePath, bool overWrite){
    return fvDropbox.uploadFile(localFile, remotePath, overWrite);
}

///Downloads a file to localFile
int FvFs::downloadFile(const QString & remotePath, QFile & localFile){
    return fvDropbox.downloadFile(remotePath, localFile);

}

int FvFs::uploadFile(const QString& localPath, bool overWrite){
    QString remotePath(fvDropbox.getAbsoluteRemotePath(fvFileWatcher.getRelativePath(localPath)));
    QFile localFile(localPath);
    return uploadFile(localFile, remotePath, overWrite);
}


///Downloads a file to the fogvaultHomeDirectory
int FvFs::downloadFile(const QString & remotePath){
    QString localPath(fvFileWatcher.getAbsolutePath(fvDropbox.getRelativeRemotePath(remotePath)));
    QFile localFile(localPath);
    return downloadFile(remotePath, localFile);
}


///Downloads a file to the specified dir
int FvFs::downloadFile(const QString & remotePath, QDir & dir){
    QString localPath(fvFileWatcher.getAbsolutePath(fvDropbox.getRelativeRemotePath(remotePath), dir));
    QFile localFile(localPath);
    return downloadFile(remotePath, localFile);
}
