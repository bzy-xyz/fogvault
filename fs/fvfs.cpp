#include "fvfs.h"

FvFs::FvFs(QObject *parent) :
    QObject(parent), fvDropbox()
{

}

FvFs::FvFs(QString homePath, FVUserKeyPair * keyPair, QObject *parent) :
    QObject(parent), fvDropbox(), fvFileWatcher(0,homePath), metadataFolder((QDir::home()).absoluteFilePath(".fogvaultmetadata")), userKeyPair(keyPair)
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

int FvFs::uploadFile(const QString& localPath, bool overWrite){
    QString remotePath(fvDropbox.getAbsoluteRemotePath(fvFileWatcher.getRelativePath(localPath)));
    QFile localFile(localPath);
    return uploadFile(localFile, remotePath, overWrite);
}

int FvFs::uploadFile(const QString& localPath, QDir & dir, bool overWrite){
    QString remotePath(fvDropbox.getAbsoluteRemotePath(fvFileWatcher.getRelativePath(localPath, dir)));
    QFile localFile(localPath);
    return uploadFile(localFile, remotePath, overWrite);
}

///Downloads a file to localFile
int FvFs::downloadFile(const QString & remotePath, QFile & localFile){
    return fvDropbox.downloadFile(remotePath, localFile);

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


int FvFs::compareMapsAndApply(QMap <QString, QDateTime>& timeMapOld, const QMap <QString, QDateTime> &timeMapNew,
                              void functionCreated(QString &), void functionModified(QString &), void functionDeleted(QString &)){
   QList <QString> keys=timeMapNew.keys();
   int i, length;
   QString key;
   QDateTime nullDate;
   length = keys.length();
   for (i=0;i<length;i++){
       key=keys[i];
       if (timeMapOld.value(key, nullDate) == nullDate){
           functionCreated(key);
       }
       else if (timeMapOld.value(key, nullDate)!= timeMapNew.value(key, nullDate)){
           functionModified(key);
           timeMapOld.remove(key);
       }
   }

   keys= timeMapOld.keys();
   length = keys.length();
   for (i=0;i<length;i++){
       key=keys[i];
       if (timeMapOld.value(key, nullDate)!= timeMapNew.value(key, nullDate)){
           functionDeleted(key);
       }
   }

   return 0;
}
 int FvFs::updateTimeMapAndApply(void functionCreated(QString &), void functionModified(QString &), void functionDeleted(QString &)){
    QMap <QString, QDateTime> oldTimeMap = fvFileWatcher.timeMap;
    fvFileWatcher.populateTimeMap();
    return compareMapsAndApply(oldTimeMap,fvFileWatcher.timeMap,functionCreated, functionModified, functionDeleted);
 }



 QMap <QString, QDateTime> FvFs::populateTimeMap(){
     return fvFileWatcher.populateTimeMap();
 }

 QString FvFs::getRelativeCriptoPath(QString & fileName){

 }

 void FvFs::createdNewFile(QString & fileName){
     QFile file(fileName);
     FVFile fvFile(file, * userKeyPair);
     QString mdFileName = fvFile.WriteMD();
     QFile mdFile(mdFileName);
     QString ctFileName=fvFile.WriteCT();
     QFile ctFile(ctFileName);
     //uploadFile(mdFile,)
 }
