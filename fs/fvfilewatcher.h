#ifndef FVFILEWATCHER_H
#define FVFILEWATCHER_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QVector>
#include <QMap>
#include <qdatetime.h>
#include <qdir.h>

class FvFileWatcher : public QObject
{
    Q_OBJECT
private:
    QFileSystemWatcher watcher;
    QVector<QString> pathVector;

    QMap<QString, QDateTime> timeMap;
    QDir fogvaulthome;
public:
    ///Constructors
    explicit FvFileWatcher(QObject *parent = 0);
    ///Constructor that adds path to file watcher
    FvFileWatcher(QObject *parent, const QString & path);

    ///Add extra paths to the file wather
    int addPath(const QString &);

    int populateTimeMap();

signals:

public slots:
    int UpdateFolder(const QString& path);
    int UpdateFile(const QString& path);
};

#endif // FVFILEWATCHER_H
