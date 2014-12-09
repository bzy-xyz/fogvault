#include "fvcontrolworker.hpp"

#include <QtDebug>

FVControlWorker::FVControlWorker(QObject *parent) :
    QObject(parent)
{
}

FVControlWorker::FVControlWorker(FvFs & fs, QObject *parent) :
    QObject(parent)
{
    this->fs = &fs;
}

void FVControlWorker::Synchronize()
{
    qDebug() << "SYNCHRONIZE";
    emit SyncDone();
}
