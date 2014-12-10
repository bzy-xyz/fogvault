#include "fvdropbox.h"
#include <qdesktopservices.h>
#include <qtextstream.h>
#include <QDir>

#include "../crypto/File.hpp"

FvDropbox::FvDropbox(QObject *parent) :
    QObject(parent), dropbox(APP_KEY, APP_SECRET), fvTokenFile(TOKENFILENAME),
    localStagingDir(QDir::temp().absoluteFilePath("FogVaultStaging"))
{
    if(!localStagingDir.exists())
    {
        QDir::temp().mkdir("FogVaultStaging");
    }
}

//Returns true if it worked. Returns false on error. Returns DROPBOX_NEED_CONFIRMATION
int FvDropbox::FvDropboxTryConnect(){
    //QFile fvTokenFile("FvToken");

    if (fvTokenFile.exists()) // has the application already been approved?
    {

        if(fvTokenFile.open(QIODevice::ReadOnly|QIODevice::Text))
        {
            QTextStream instream(&fvTokenFile);
            QString token = instream.readLine().trimmed();
            QString secret = instream.readLine().trimmed();
            if(!token.isEmpty() && !secret.isEmpty())
            {
                dropbox.setToken(token);
                dropbox.setTokenSecret(secret);
                fvTokenFile.close();
                return 1;
            }
        }//If it exists but can't be read, treat as if it doesn't exist
        fvTokenFile.close();
    }
    //If we can't request the Request token from dropbox, possibly because of key problems:
    if(!dropbox.requestTokenAndWait())
    {
        throw FvFsDropboxRequestTokenException();
        return false;
    }
    dropbox.setAuthMethod(QDropbox::Plaintext);
    if(!dropbox.requestAccessTokenAndWait()){  //If the account is not already connected to DB
        QDesktopServices::openUrl(dropbox.authorizeLink());
        return DROPBOX_NEED_CONFIRMATION;
    }
    else{
        return saveTokenToDisk();
    }

}

int FvDropbox::FvDropboxFinishConnecting(){

    if (dropbox.requestAccessTokenAndWait()){
        saveTokenToDisk();
        return 1;
    }
    throw FvFsDropboxRequestTokenException("error requesting access token");
    return false;
}
int FvDropbox::saveTokenToDisk(){
    //if(!fvTokenFile.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text))
        return false;


    QTextStream saveStream(&fvTokenFile);
    saveStream << dropbox.token() << endl;
    saveStream << dropbox.tokenSecret() << endl;
    fvTokenFile.close();
    return 1;
}

int FvDropbox::uploadFile(QFile & localFile, const QString& remotePath, bool overWrite){
    QDropboxFile dropboxFile(remotePath,&dropbox);
    dropboxFile.setOverwrite(overWrite);
    if(!dropboxFile.open(QDropboxFile::WriteOnly)){
        //error opening the file
        throw FvFsDropboxFileException("Error opening write only remote file");
    }

    if(!localFile.open(QDropboxFile::ReadOnly)){
        //error opening the file
        throw FvFsDropboxFileException("Error opening read only local file");
    }

    dropboxFile.write(localFile.readAll());
    localFile.close();
    dropboxFile.close();
    return 0;


}

int FvDropbox::downloadFile(const QString & remotePath, QFile & localFile){
    QDropboxFile dropboxFile(remotePath,&dropbox);
    if(!dropboxFile.open(QDropboxFile::ReadOnly)){
        //error opening the file
        throw FvFsDropboxFileException("Error opening read only remote file");
    }

    if(!localFile.open(QDropboxFile::WriteOnly)){
        //error opening the file
        throw FvFsDropboxFileException("Error opening read only local file");
    }

    localFile.write(dropboxFile.readAll());
    localFile.close();
    dropboxFile.close();
    return 0;
}

QString FvDropbox::getAbsoluteRemotePath(const QString& relativePath){
    return (DROPBOX_PATH_PREFIX+relativePath);
}

QString FvDropbox::getRelativeRemotePath(const QString& absolutePath){
    if (!(absolutePath.startsWith(DROPBOX_PATH_PREFIX))){
            throw FvFsDropboxPathException("path not inside app folder(" DROPBOX_PATH_PREFIX "), path= %s", absolutePath.toStdString().c_str());
    }
    return absolutePath.mid(DROPBOX_PATH_PREFIX_LENGTH);

}

void FvDropbox::UpdateRemoteState()
{
    qDebug() << "UPDATING REMOTE STATE";
    bool hasMore = true;
    do
    {
        QDropboxDeltaResponse r = dropbox.requestDeltaAndWait(remoteCursor, "");
        remoteCursor = r.getNextCursor();
        hasMore = r.hasMore();

        const QDropboxDeltaEntryMap entries = r.getEntries();
        for(QDropboxDeltaEntryMap::const_iterator i = entries.begin(); i != entries.end(); i++)
        {
            if(i.key().right(FOGVAULT_FILE_MD_EXTENSION_LENGTH) == FOGVAULT_FILE_MD_EXTENSION
            || i.key().right(FOGVAULT_FILE_CTX_EXTENSION_LENGTH) == FOGVAULT_FILE_CTX_EXTENSION)
            {
                if(i.value().isNull())
                {
                    remoteInfo.remove(i.key());

                    emit RemoteFileRemoved(i.key());
                }
                else
                {
                    remoteInfo.insert(i.key(), i.value());

                    if(i.value()->isDir())
                    {
                        emit RemoteDirAvailable(i.key());
                    }
                    else
                    {
                        emit RemoteFileAvailable(i.key());
                    }
                }
            }
        }

    } while (hasMore);

}

void FvDropbox::DownloadAndStageFile(const QString dbxPath)
{
    QString localPath = localStagingDir.absoluteFilePath(dbxPath.right(dbxPath.length()-1));
    QFile localFile(localPath);

    this->downloadFile(this->getAbsoluteRemotePath(dbxPath), localFile);

    emit RemoteFileStagedLocally(localPath, dbxPath);
}
