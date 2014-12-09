#ifndef FVCONTROLWORKER_HPP
#define FVCONTROLWORKER_HPP

#include <QObject>
#include "fs/fvdropbox.h"
#include "fs/fvfilewatcher.h"

class FVControlWorker : public QObject
{
    Q_OBJECT
public:
    explicit FVControlWorker(QObject *parent = 0);

    FVControlWorker(FvDropbox & dbx, FvFileWatcher & fw, QObject * parent = 0);

signals:
    void SyncDone();

public slots:
    void Synchronize();

private:
    FvDropbox * dbx;
    FvFileWatcher * fw;
};

#endif // FVCONTROLWORKER_HPP
