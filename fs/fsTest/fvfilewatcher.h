#ifndef FVFILEWATCHER_H
#define FVFILEWATCHER_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QVector>
#include <QMap>
class FvFileWatcher : public QObject
{
    Q_OBJECT
private:
    QFileSystemWatcher watcher;
    QVector<QString> pathVector;
    //QVector<QString> pathVector;
public:
    ///Constructors
    explicit FvFileWatcher(QObject *parent = 0);
    ///Constructor that adds path to file watcher
    FvFileWatcher(QObject *parent, char *path);

    ///Add extra paths to the file wather
    int addPath(char *path);
signals:

public slots:
    int UpdateFolder(const QString& path);
    int UpdateFile(const QString& path);
};

#endif // FVFILEWATCHER_H