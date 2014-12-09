#include "fvcontrolworker.hpp"

#include <QtDebug>

FVControlWorker::FVControlWorker(QObject *parent) :
    QObject(parent)
{
}

FVControlWorker::FVControlWorker(FvDropbox &dbx, FvFileWatcher &fw, QObject *parent) :
    QObject(parent)
{
    this->dbx = &dbx;
    this->fw = &fw;
}

void FVControlWorker::Synchronize()
{
    qDebug() << "SYNCHRONIZE";
    emit SyncDone();
}
