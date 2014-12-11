#ifndef FVFS_H
#define FVFS_H

#include <QObject>
#include <QDir>
#include "fvdropbox.h"
#include "fvfilewatcher.h"
#include "../crypto/File.hpp"

class FvFs : public QObject
{
    Q_OBJECT
private:
    FvDropbox fvDropbox;
    FvFileWatcher fvFileWatcher;
    QDir metadataFolder;
    FVUserKeyPair * userKeyPair;
    void createdNewFile(QString & fileName);
    void createdNewFolder(QString & fileName);
    void modifiedFile(QString & fileName);

    void null(QString & fileName);
    QMap <QString, QString> pt2ct;
    QMap <QString, QString> ct2pt;
public:
    explicit FvFs(QObject *parent = 0);
    FvFs(QString homePath, FVUserKeyPair * keyPair, QObject *parent = 0);

    int testUpdateTimeMapAndApply();
    FvDropbox & GetFvDropboxRef()
    {
        return fvDropbox;
    }

    FvFileWatcher & GetFvFileWatcherRef()
    {
        return fvFileWatcher;
    }

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
    int uploadFile(const QString& localPath, QDir & dir, bool overWrite);
    ///Downloads a file to localFile
    int downloadFile(const QString & remotePath, QFile & localFile);
    ///Downloads a file to the fogvaultHomeDirectory
    int downloadFile(const QString & remotePath);
    ///Downloads a file to the specifiedDir
    int downloadFile(const QString & remotePath, QDir & dir);

    ///Compares both time maps and applies function on the QString canonical path of the files
    /// that have been modified, deleted or created.
    /// It modifies

    ///
    /// \brief compareMapsAndApply Compares both time maps and applies function on the QString canonical path of the files
    /// that have been modified, deleted or created.
    /// \param timeMapOld - The old version of timeMap. WILL BE MODIFIED and will contain only the files that have been deleted.
    /// \param timeMapNew - The new version of the time map. Will not be modified.
    /// \param function(action) - Function to be applied when action happened to file
    /// \return 0 on success.
    ///
    int compareMapsAndApply(QMap <QString, QDateTime>& timeMapOld, const QMap <QString, QDateTime> &timeMapNew,
                            void functionCreated(QString &), void functionModified(QString &), void functionDeleted(QString &));
    int updateTimeMapAndApply(void functionCreated(QString &), void functionModified(QString &), void functionDeleted(QString &));

    int compareMapsAndApply(QMap <QString, QDateTime>& timeMapOld, const QMap <QString, QDateTime> &timeMapNew);
    int updateTimeMapAndApply();

    ///
    /// \brief populateTimeMap: first populate the time Map
    /// \return
    ///
    QMap <QString, QDateTime> populateTimeMap();

    QString getRelativeCriptoPath(QString & fileName);

    QString getRelativePlainPath(QString & relativeCriptoPath);


    QString merge2Path(QStringList paths, int index);

    void addDownloadedFile(QString & fileName);
signals:

public slots:
    ///
    /// \brief FvDropboxFinishConnecting to be called after FvDropboxTryConnect() after the user accepted to connect the app to DB.
    /// \return  Returns 1 if it manages to connect and save the token to disk. Returns 0 if it didn't manage to save the token to disk
    ///
    int FvDropboxFinishConnecting();

};

#endif // FVFS_H
