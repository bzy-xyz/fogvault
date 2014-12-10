#ifndef FVCONTROLWORKER_HPP
#define FVCONTROLWORKER_HPP

#include <QObject>
#include <QMap>
#include "crypto/File.hpp"
#include "crypto/UserKey.hpp"
#include "fs/fvfs.h"
#include "qtdropbox/qdropboxdeltaresponse.h"

struct fv_control_entry_t
{
    QString fv_pt_path;

    bool is_directory;
    bool is_deleted;

    QString fv_dbx_path_base;
    QString fv_dbx_path_ct()
    {
        return fv_dbx_path_base + FOGVAULT_FILE_CTX_EXTENSION;
    }

    QString fv_dbx_path_md()
    {
        return fv_dbx_path_base + FOGVAULT_FILE_MD_EXTENSION;
    }

    bool fv_dbx_ct_exists;
    bool fv_dbx_md_exists;

    QString fv_ctl_path_md;
    QString fv_staging_path_ct;
    QString fv_staging_path_md;

    bool needsUpload;
    bool needsDownload;
};

struct fv_control_state_t
{
    FVUserKeyPair * kp;

    // TODO: the correct tool for this job is a database,
    // but we're kinda out of options here
    QMap<QString, QSharedPointer<fv_control_entry_t> > idx_pt_path;
    QMap<QString, QSharedPointer<fv_control_entry_t> > idx_ctl_md_path;
    QMap<QString, QSharedPointer<fv_control_entry_t> > idx_dbx_path_base;

    QSharedPointer<fv_control_entry_t> by_pt_path(const QString p)
    {
        if(idx_pt_path.contains(p))
        {
            return idx_pt_path[p];
        }
        QSharedPointer<fv_control_entry_t> ret (new fv_control_entry_t);
        ret->fv_pt_path = p;
        idx_pt_path.insert(p, ret);
        ret->fv_dbx_ct_exists = false;
        ret->fv_dbx_md_exists = false;
        ret->needsDownload = false;
        ret->needsUpload = false;
        ret->is_deleted = false;
        ret->is_directory = false;
        return ret;
    }

    QSharedPointer<fv_control_entry_t> by_ctl_md_path(const QString p)
    {
        if(idx_ctl_md_path.contains(p))
        {
            return idx_ctl_md_path[p];
        }
        QSharedPointer<fv_control_entry_t> ret (new fv_control_entry_t);
        ret->fv_ctl_path_md = p;
        ret->fv_dbx_ct_exists = false;
        ret->fv_dbx_md_exists = false;
        ret->needsDownload = false;
        ret->needsUpload = false;

        QFile mdf(p);
        FVFile f(mdf, 0, *this->kp);

        ret->is_deleted = f.IsDeleted();
        ret->is_directory = f.IsDirectory();

        idx_ctl_md_path.insert(p, ret);

        return ret;
    }

    QSharedPointer<fv_control_entry_t> by_dbx_path_base(const QString p)
    {
        if(idx_dbx_path_base.contains(p))
        {
            return idx_dbx_path_base[p];
        }
        QSharedPointer<fv_control_entry_t> ret (new fv_control_entry_t);
        ret->fv_dbx_path_base = p;
        idx_pt_path.insert(p, ret);
        ret->fv_dbx_ct_exists = false;
        ret->fv_dbx_md_exists = false;
        ret->needsDownload = false;
        ret->needsUpload = false;
        ret->is_deleted = false;
        ret->is_directory = false;
        return ret;
    }
};

class FVControlWorker : public QObject
{
    Q_OBJECT
public:
    explicit FVControlWorker(QObject *parent = 0);

    FVControlWorker(FvFs & fs, FVUserKeyPair & k, QObject * parent = 0);

signals:
    void SyncDone();

public slots:
    void Synchronize();

    void HandleDropboxFileRemoved(const QString dbxPath);
    void HandleDropboxFileStagedLocally(const QString stagingPath, const QString dbxPath);

private:
    FvFs * fs;
    FVUserKeyPair k;

    QDropboxFileInfoMap dbx_state;
    fv_control_state_t ctl_state;

};

#endif // FVCONTROLWORKER_HPP
