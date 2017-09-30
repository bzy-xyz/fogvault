#ifndef FVCONTROL_HPP
#define FVCONTROL_HPP

#include <QObject>
#include <QTimer>
#include <QThread>
#include "fvcontrolworker.hpp"
#include "fs/fvdropbox.h"
#include "fs/fvfilewatcher.h"

#define TIMEOUT_INTERVAL 5000

class FVControl : public QObject
{
    Q_OBJECT
public:
    explicit FVControl(QObject *parent = 0);
    FVControl(FvFs & fs, QObject * parent = 0);

    ~FVControl();

    void start(FVUserKeyPair & k);

signals:
    void Synchronize();

public slots:
    void HandleSyncDone();
    void DoSynchronize();

private:
    QThread workerThread;
    QTimer syncTimer;
    FvFs * fs;
};

#endif // FVCONTROL_HPP
