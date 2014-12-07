#include "fvfilewatcher.h"

FvFileWatcher::FvFileWatcher(QObject *parent) :
    QObject(parent)
{
}


/// Add the path to the list of watched directories
FvFileWatcher::FvFileWatcher(QObject *parent, const QString & path) :
        QObject(parent)
{
    addPath(path);
    QObject::connect(&watcher, SIGNAL(directoryChanged(QString)), this, SLOT(UpdateFolder(QString)));
    QObject::connect(&watcher, SIGNAL(fileChanged(QString)), this, SLOT(UpdateFile(QString)));

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
