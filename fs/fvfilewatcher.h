#ifndef FVFILEWATCHER_H
#define FVFILEWATCHER_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QVector>
#include <QMap>
#include <qdatetime.h>
#include <qdir.h>
#include <qdiriterator.h>
#include "fvfsexceptions.h"

class FvFileWatcher : public QObject
{
    Q_OBJECT
private:
    QFileSystemWatcher watcher;
    QVector<QString> pathVector;

public:
    QDir fogvaulthome;
    QMap<QString, QDateTime> timeMap;
    ///Constructors
    explicit FvFileWatcher(QObject *parent = 0);
    ///Constructor that adds path to file watcher
    FvFileWatcher(QObject *parent, const QString & path);
    FvFileWatcher(QObject *parent, QDir & home);


    ///Add extra paths to the file watcher
    int addPath(const QString &);

    ///
    /// \brief populateTimeMap
    /// \return
    ///
    QMap <QString, QDateTime> populateTimeMap(QMap<QString, QDateTime>& timeMap, QDir& fogvaulthome);
    QMap <QString, QDateTime> populateTimeMap();

    ///Returns the path of the file removing the fogVaultHome path from it
    /// returns NULL if the file is not inside fogVaultHome
    QString getRelativePath(const QString & absolutePath, QDir & folder);

    ///TODO
    QString getRelativePath(const QString & absolutePath);

    ///
    /// \brief getAbsolutePath adds fog vault home path to the beginer of the path
    /// \param relativePath
    /// \return returns the absolute path
    ///
    QString getAbsolutePath(const QString & relativePath, QDir& folder);

    ///TODO
    QString getAbsolutePath(const QString & relativePath);


signals:

public slots:
    int UpdateFolder(const QString& path);
    int UpdateFile(const QString& path);
};

#endif // FVFILEWATCHER_H
