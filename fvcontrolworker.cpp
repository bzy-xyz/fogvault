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
    this->connect(&dbx,
                  &FvDropbox::RemoteDirAvailable,
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

    qDebug() << dbxPath << " received from Dropbox -> " << stagingPath;

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

        QFile mdf(stagingPath);
        FVFile fvf(mdf, 0, *this->ctl_state.kp);
        e->is_deleted = fvf.IsDeleted();
        e->is_directory = fvf.IsDirectory();
    }

    // Do we have enough information from Dropbox to make a decision as to what to do?
    // in particular:
    // - If I know I am an ordinary file then I need both a ct and a md before acting
    // - If I know I am a directory then I need only a md before acting
    // TODO: care about file deletion
    if(e->is_directory && e->fv_dbx_md_exists)
    {
        qDebug() << "TODO: handle extant directory";


        QFile mdf(e->fv_staging_path_md);
        FVFile f(mdf,0,*this->ctl_state.kp);

        QString dirname = f.PTFileName();

        // transform the ciphertext path to a plaintext path
        // and create a directory at the correct place
        QDir staging_home(QDir::temp().absoluteFilePath("FogVaultStaging"));

        QFileInfo staging_fi(stagingPath);
        QDir staging_dir = staging_fi.dir();
        QString relativePath = staging_home.relativeFilePath(staging_dir.absolutePath());
        QString relativePathWithCryptDirname = relativePath + (relativePath == "" || relativePath.right(1) == "/" ? "" : "/") + f.CTFileName();
        QString actualRelativePath = fs->getRelativePlainPath(relativePath);
        QString actualRelativePathWithDirname = actualRelativePath + (actualRelativePath == "" || actualRelativePath.right(1) == "/" ? "" : "/") + dirname;

        QDir out_md = (QDir(QDir::home().absoluteFilePath(".fogvaultmetadata")).absoluteFilePath(actualRelativePath));
        if(!out_md.exists())
        {
            staging_home.mkpath(out_md.path());
        }
        f.WriteMD(out_md, false);

        QDir home_dir(QDir::home().absoluteFilePath("FogVault"));
        QDir out_dir(QDir(home_dir.absoluteFilePath(actualRelativePath)).absoluteFilePath(dirname));
        if(!out_dir.exists())
        {
            home_dir.mkpath(out_dir.absolutePath());
        }

        this->fs->addCriptoToPlainMapping(relativePathWithCryptDirname,actualRelativePathWithDirname);
        QString pathToAdd(out_dir.absolutePath());
        this->fs->addDownloadedFile(pathToAdd);
    }
    else if(e->fv_dbx_md_exists && e->fv_dbx_ct_exists)
    {
        // We now have to write out a file to the local directory
        QFile mdf(e->fv_staging_path_md);
        QFile ctf(e->fv_staging_path_ct);
        FVFile f(mdf, ctf, *this->ctl_state.kp);

        // TODO: do this properly
        //QDir tmp = QDir::temp();
        //QDir out_tmp = tmp.absoluteFilePath("FogVaultTestOutput");

        // HACK X( get relative path from staging root
        // this is bad software engineering, staging path should really be retrievable
        // from FvDropbox or some global config manager
        QDir staging_home(QDir::temp().absoluteFilePath("FogVaultStaging"));

        QFileInfo staging_fi(stagingPath);
        QDir staging_dir = staging_fi.dir();
        QString relativePath = staging_home.relativeFilePath(staging_dir.absolutePath());
        QString actualRelativePath = fs->getRelativePlainPath(relativePath);

        QDir out_pt(QDir(QDir::home().absoluteFilePath("FogVault")).absoluteFilePath(actualRelativePath));
        if(!out_pt.exists())
        {
            staging_home.mkpath(out_pt.path());
        }
        QDir out_md = (QDir(QDir::home().absoluteFilePath(".fogvaultmetadata")).absoluteFilePath(actualRelativePath));
        if(!out_md.exists())
        {
            staging_home.mkpath(out_md.path());
        }

        /*if(!out_tmp.exists())
        {
            tmp.mkdir("FogVaultTestOutput");
        }*/
        QString c = f.WritePT(out_pt);
        f.WriteMD(out_md, false);
        this->fs->addDownloadedFile(c);
        qDebug() << "wrote file " << c;

        staging_home.remove(e->fv_staging_path_ct);
        staging_home.remove(e->fv_staging_path_md);
        e->fv_staging_path_ct = "";
        e->fv_staging_path_md = "";
        e->fv_dbx_ct_exists = false;
        e->fv_dbx_md_exists = false;
    }
}

void FVControlWorker::HandleDropboxDirAdded(const QString dbxPath)
{
    // Here a dir was created on dbx.
    // Update the corresponding control entry to add the directory.

    qDebug() << dbxPath << "directory created on Dropbox";

    QSharedPointer<fv_control_entry_t> e = this->ctl_state.by_dbx_path_base(dbxPath);

    e->is_directory = true;

    if(e->fv_dbx_md_exists)
    {
        qDebug() << "TODO: handle extant directory";
        // TODO:
        // transform the ciphertext path to a plaintext path
        // and create a directory at the correct place
        QFile mdf(e->fv_staging_path_md);
        FVFile f(mdf,0,*this->ctl_state.kp);

        QString dirname = f.PTFileName();
        // TODO:
        // transform the ciphertext path to a plaintext path
        // and create a directory at the correct place
    }

}

void FVControlWorker::Synchronize()
{
    qDebug() << "SYNCHRONIZE";


    this->fs->GetFvDropboxRef().UpdateRemoteState();
    this->fs->updateTimeMapAndApply();

    emit SyncDone();
}
