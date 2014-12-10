#include "fvcontrolworker.hpp"
#include "fs/fvdropbox.h"

#include <QtDebug>

FVControlWorker::FVControlWorker(QObject *parent) :
    QObject(parent)
{
}

FVControlWorker::FVControlWorker(FvFs & fs, FVUserKeyPair & k, QObject *parent) :
    QObject(parent), k(k)
{
    this->fs = &fs;
    this->ctl_state.kp = &k;

    // TODO:
    // We need to load up both the remote Dropbox state
    // and the local filesystem image,
    // and then reconcile the two.
    // Only then should we wire up the relevant FvDropbox signals.

    // Because this is a demo,
    // we'll play this one fast and loose and hacky
    // and assume we start with an *empty* local directory...

    FvDropbox & dbx = this->fs->GetFvDropboxRef();
    this->connect(&dbx,
                  &FvDropbox::RemoteFileAvailable,
                  &dbx,
                  &FvDropbox::DownloadAndStageFile);
    this->connect(&dbx,
                  &FvDropbox::RemoteFileStagedLocally,
                  this,
                  &FVControlWorker::HandleDropboxFileStagedLocally);
    this->connect(&dbx,
                  &FvDropbox::RemoteFileRemoved,
                  this,
                  &FVControlWorker::HandleDropboxFileRemoved);
}

void FVControlWorker::HandleDropboxFileRemoved(const QString dbxPath)
{
    // Here a DBX file was removed.
    //
    qDebug() << dbxPath << " removed from Dropbox";
}

void FVControlWorker::HandleDropboxFileStagedLocally(const QString stagingPath, const QString dbxPath)
{
    // Here a DBX file was received and staged locally.
    // Update the corresponding control entry to add the locally staged file.

    QSharedPointer<fv_control_entry_t> e = this->ctl_state.by_dbx_path_base(
                                               dbxPath.left(dbxPath.length() - FOGVAULT_FILE_CTX_EXTENSION_LENGTH)
                                           );

    QString ext = dbxPath.right(FOGVAULT_FILE_CTX_EXTENSION_LENGTH);
    if(ext == FOGVAULT_FILE_CTX_EXTENSION)
    {
        e->fv_dbx_ct_exists = true;
        e->fv_staging_path_ct = stagingPath;
    }
    else if(ext == FOGVAULT_FILE_MD_EXTENSION)
    {
        e->fv_dbx_md_exists = true;
        e->fv_staging_path_md = stagingPath;
    }
}

void FVControlWorker::Synchronize()
{
    qDebug() << "SYNCHRONIZE";


    this->fs->GetFvDropboxRef().UpdateRemoteState();

    emit SyncDone();
}
