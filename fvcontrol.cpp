#include "fvcontrol.hpp"

#include "fvcontrolworker.hpp"

FVControl::FVControl(QObject *parent) :
    QObject(parent)
{
}

FVControl::FVControl(FvFs & fs, QObject * parent) :
    QObject(parent), syncTimer(this)
{
    FVControlWorker * w = new FVControlWorker(fs, parent);
    w->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, w, &QObject::deleteLater);
    connect(this, &FVControl::Synchronize, w, &FVControlWorker::Synchronize);
    connect(w, &FVControlWorker::SyncDone, this, &FVControl::HandleSyncDone);

    connect(&syncTimer, &QTimer::timeout, this, &FVControl::DoSynchronize);

    workerThread.start();
    syncTimer.start(TIMEOUT_INTERVAL);
    qDebug() << "START";
}

FVControl::~FVControl()
{
    syncTimer.stop();
    workerThread.quit();
    workerThread.wait();
}

void FVControl::HandleSyncDone()
{

}

void FVControl::DoSynchronize()
{
    emit Synchronize();
}
