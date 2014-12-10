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

 QString FvFs::getRelativeCriptoPath(QString & fileNameAndPath){
     QChar separator =QDir::separator();
     QStringList paths;
     QString relativePathName=fvFileWatcher.getRelativePath(fileNameAndPath);
     QString relativeCriptoPath("");
     int i;
     paths=relativePathName.split(separator);
     int length=paths.length();
     for (i=0;i<length;i++){
         relativeCriptoPath.append(pt2ct.value(merge2Path(paths,i)));
     }
     return relativeCriptoPath;
 }

 QString FvFs::merge2Path(QStringList paths, int index){
     QChar separator =QDir::separator();
     int i;
     QString path(paths[0]);
     for (i=1;i<index;i++){
         path.append(separator);
         path.append(paths[i]);
     }
     return path;
 }
 void FvFs::null(QString & fileName){

 }

 void FvFs::createdNewFile(QString & fileName){
     QFileInfo fileInfo(fileName);
     if (fileInfo.isDir()){
         return createdNewFolder(fileName);
     }
     QFile file(fileName);
     FVFile fvFile(file, * userKeyPair);
     QString mdFileName = fvFile.WriteMD();
     QFile mdFile(mdFileName);
     QString ctFileName=fvFile.WriteCT();
     QFile ctFile(ctFileName);
     //ns uploadFile(mdFile,)
 }
void FvFs::createdNewFolder(QString & fileName){
    QFileInfo fileInfo(fileName);
    if (fileInfo.isDir()){
        QDir dir(fileName);
        FVFile fvFile(dir, * userKeyPair);
        QString relativePath= fvFileWatcher.getRelativePath(fileInfo.absolutePath());
        QDir folder(metadataFolder.filePath(relativePath));
        QString mdFilePath = fvFile.WriteMD(folder,false);
        QFile mdFile(mdFilePath);
        QString ctFilePath=fvFile.WriteCT();
        QFile ctFile(ctFilePath);
        QString criptoRelativePath = getRelativeCriptoPath(fileName);
        pt2ct.insert(relativePath,criptoRelativePath);
        ct2pt.insert(criptoRelativePath,relativePath);
    }

}
