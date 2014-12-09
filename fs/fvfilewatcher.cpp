#include "fvfilewatcher.h"
FvFileWatcher::FvFileWatcher(QObject *parent) :
    QObject(parent)
{
}


FvFileWatcher::FvFileWatcher(QObject *parent, const QString & path) :
        QObject(parent), fogvaulthome(path)
{

}

FvFileWatcher::FvFileWatcher(QObject *parent, QDir & home) :
        QObject(parent), fogvaulthome(home)
{

}


QString FvFileWatcher::getAbsolutePath(const QString & relativePath, QDir & folder){
    return (folder.absoluteFilePath(relativePath));
}

QString FvFileWatcher::getAbsolutePath(const QString & relativePath){
    return (getAbsolutePath(relativePath, fogvaulthome));
}

QString FvFileWatcher::getRelativePath(const QString & absolutePath, QDir & folder){
    return folder.relativeFilePath(absolutePath);
}

QString FvFileWatcher::getRelativePath(const QString & absolutePath){
    return getRelativePath(absolutePath, fogvaulthome);
}

QMap<QString, QDateTime> FvFileWatcher::populateTimeMap(QMap<QString, QDateTime>& timeMap, QDir& folder){
    folder.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks |QDir::Dirs);
    QFileInfoList files(folder.entryInfoList());
    QFileInfo fileInfo;
    int i, length;
    length=files.length();
    for (i=0; i<length;i++){
        fileInfo=files[i];
        if (!(fileInfo.isDir())){
            timeMap.insert(fileInfo.canonicalFilePath() ,fileInfo.lastModified());
        }
        else{
            QDir dir(fileInfo.canonicalFilePath());
            if (dir != fogvaulthome){}
                populateTimeMap(timeMap, dir);
        }

    }
 return timeMap;
}


QMap<QString, QDateTime> FvFileWatcher::populateTimeMap(){
    QMap<QString, QDateTime> newTimeMap;
    populateTimeMap(newTimeMap, fogvaulthome);
    timeMap=newTimeMap;
    return timeMap;
}

///Update stuff on Dropbox + find what changed when receiving a directory change
int FvFileWatcher::UpdateFolder(const QString& path){
return 0;
}

///Update stuff on Dropbox + find what changed when receiving a file change
int FvFileWatcher::UpdateFile(const QString& path){

return 0;
}

int FvFileWatcher::addPath(const QString & path){
    //pathVector.append(QString::fromStdString(path));
    watcher.addPath(path);
    return 0;
}
