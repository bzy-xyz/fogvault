#include "fvfs.h"
#include <QTemporaryDir>


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

int FvFs::compareMapsAndApply(QMap <QString, QDateTime>& timeMapOld, const QMap <QString, QDateTime> &timeMapNew){
   QList <QString> keys=timeMapNew.keys();
   int i, length;
   QString key;
   QDateTime nullDate;
   length = keys.length();
   for (i=0;i<length;i++){
       key=keys[i];
       if (timeMapOld.value(key, nullDate) == nullDate){
           createdNewFile(key);
       }
       else if (timeMapOld.value(key, nullDate)!= timeMapNew.value(key, nullDate)){
           modifiedFile(key);
           timeMapOld.remove(key);
       }
   }

   keys= timeMapOld.keys();
   length = keys.length();
   for (i=0;i<length;i++){
       key=keys[i];
       if (timeMapOld.value(key, nullDate)!= timeMapNew.value(key, nullDate)){
           null(key);
       }
   }

   return 0;
}

 int FvFs::updateTimeMapAndApply(void functionCreated(QString &), void functionModified(QString &), void functionDeleted(QString &)){
    QMap <QString, QDateTime> oldTimeMap = fvFileWatcher.timeMap;
    fvFileWatcher.populateTimeMap();
    return compareMapsAndApply(oldTimeMap,fvFileWatcher.timeMap,functionCreated, functionModified, functionDeleted);
 }

 int FvFs::updateTimeMapAndApply(){
    QMap <QString, QDateTime> oldTimeMap = fvFileWatcher.timeMap;
    fvFileWatcher.populateTimeMap();
    return compareMapsAndApply(oldTimeMap,fvFileWatcher.timeMap);
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

     QFile dir(fileName);
     FVFile fvFile(dir, * userKeyPair);
     //Gets the relative path of the file, to find where to put it inside the metada folder
     QString relativePath= fvFileWatcher.getRelativePath(fileInfo.canonicalPath());
     QDir folder(metadataFolder.filePath(relativePath));

     //Creates the metadata file inside the metadata folder
     QString mdFilePath = fvFile.WriteMD(folder,false);
     QFile mdFile(mdFilePath);



     //Creates a temp folder for the files that will be uploaded
     QString filePath(QFileInfo(fileName).canonicalPath());
     QTemporaryDir tmpDir;
     QDir tempDir(tmpDir.path());
     QString criptoRelativePath = getRelativeCriptoPath(filePath);
     QFileInfo tempInfo(tempDir.absoluteFilePath(criptoRelativePath));
     QDir tempCriptoDir(tempInfo.absoluteFilePath()); //Gets the path of the parent dir
     tempCriptoDir.mkpath(tempInfo.absoluteFilePath()); //creates the path

     //Creates and upload the ctFile
     QString ctFilePath=fvFile.WriteCT(tempCriptoDir);
     uploadFile(ctFilePath,tempDir,true);

     //Creates and upload the mdFile
     QString mdFilePathCript=fvFile.WriteMD(tempCriptoDir);
     QFileInfo mdFileInfo(mdFilePathCript);
     uploadFile(mdFilePathCript,tempDir, true);


     //Now add everything to our maps that are used on getRelativeCriptoPath, among other functions
     QString criptoRelativeName = criptoRelativePath + mdFileInfo.baseName();
     QString relativePathName = fvFileWatcher.getRelativePath(fileInfo.absoluteFilePath());
     pt2ct.insert(relativePathName,criptoRelativeName);
     ct2pt.insert(criptoRelativeName,relativePathName);
 }


 void FvFs::createdNewFolder(QString & fileName){
    QFileInfo fileInfo(fileName);
    if (fileInfo.isDir()){
        //Finds the path to the parent directory

        QDir dir(fileName);
        FVFile fvFile(dir, * userKeyPair);
        //Gets the relative path of the file, to find where to put it inside the metada folder
        QString relativePath= fvFileWatcher.getRelativePath(fileInfo.canonicalPath());
        QDir folder(metadataFolder.filePath(relativePath));

        folder.mkpath(metadataFolder.filePath(relativePath));

        //Creates the metadata file inside the metadata folder
        QString mdFilePath = fvFile.WriteMD(folder,false);
        QFile mdFile(mdFilePath);



        //Creates a temp folder for the files that will be uploaded
        QString filePath(QFileInfo(fileName).canonicalPath());
        QTemporaryDir tmpDir;
        QDir tempDir(tmpDir.path());
        QString criptoRelativePath = getRelativeCriptoPath(filePath);
        QFileInfo tempInfo(tempDir.absoluteFilePath(criptoRelativePath));
        QDir tempCriptoDir(tempInfo.absoluteFilePath()); //Gets the path of the parent dir
        tempCriptoDir.mkpath(tempInfo.absoluteFilePath()); //creates the path

        //Creates and upload the ctFile
    //    QString ctFilePath=fvFile.WriteCT(tempCriptoDir);
      //  uploadFile(ctFilePath,tempCriptoDir);

        //Creates and upload the mdFile
        QString mdFilePathCript=fvFile.WriteMD(tempCriptoDir);
        QFileInfo mdFileInfo(mdFilePathCript);
        uploadFile(mdFilePathCript,tempDir, true);


        //Now add everything to our maps that are used on getRelativeCriptoPath, among other functions
        QString criptoRelativeName = criptoRelativePath + mdFileInfo.baseName();
        QString relativePathName = fvFileWatcher.getRelativePath(fileInfo.absoluteFilePath());
        pt2ct.insert(relativePathName,criptoRelativeName);
        ct2pt.insert(criptoRelativeName,relativePathName);

    }

}

 void FvFs::modifiedFile(QString & fileName){
     QFileInfo fileInfo(fileName);
     if (fileInfo.isDir()){
         return createdNewFolder(fileName);
     }

     QFile file(fileName);
     QString relativeName= fvFileWatcher.getRelativePath(fileInfo.canonicalFilePath());
     QString relativeMdName = relativeName+".fvm";
     QFile mdFile(metadataFolder.filePath(relativeMdName));


     FVFile fvFile(mdFile, file, * userKeyPair, NULL, false);



     //Creates a temp folder for the files that will be uploaded
     QString filePath(QFileInfo(fileName).canonicalPath());
     QTemporaryDir tmpDir;
     QDir tempDir(tmpDir.path());
     QString criptoRelativePath = getRelativeCriptoPath(filePath);
     QFileInfo tempInfo(tempDir.absoluteFilePath(criptoRelativePath));
     QDir tempCriptoDir(tempInfo.canonicalPath()); //Gets the path of the parent dir
     tempCriptoDir.mkpath(tempCriptoDir.canonicalPath()); //creates the path

     //Creates and upload the ctFile
     QString ctFilePath=fvFile.WriteCT(tempCriptoDir);
     uploadFile(ctFilePath,tempCriptoDir,true);

     //Creates and upload the mdFile
     QString mdFilePathCript=fvFile.WriteMD(tempCriptoDir);
     QFileInfo mdFileInfo(mdFilePathCript);
     uploadFile(mdFilePathCript,tempCriptoDir, true);
}
  void FvFs::addDownloadedFile(QString & fileName){

     QFileInfo fileInfo(fileName);

     fvFileWatcher.timeMap.remove(fileInfo.canonicalFilePath());

     fvFileWatcher.timeMap.insert(fileInfo.canonicalFilePath(), fileInfo.lastModified());

 }
