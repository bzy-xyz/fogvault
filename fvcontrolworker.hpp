#ifndef FVCONTROLWORKER_HPP
#define FVCONTROLWORKER_HPP

#include <QObject>
#include "fs/fvfs.h"

class FVControlWorker : public QObject
{
    Q_OBJECT
public:
    explicit FVControlWorker(QObject *parent = 0);

    FVControlWorker(FvFs & fs, QObject * parent = 0);

signals:
    void SyncDone();

public slots:
    void Synchronize();

private:
    FvFs * fs;
};

#endif // FVCONTROLWORKER_HPP
