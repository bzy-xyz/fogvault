#include <QString>
#include <QtTest>
#include <fvdropbox.h>
#include <fvfilewatcher.h>
#include<../../qtdropbox/qdropbox.h>
#include <cstdio>
#include <iostream>

class FsTest : public QObject
{
    Q_OBJECT

public:
    FsTest();

private Q_SLOTS:
    void testCase1();
    void testDropbox();
    void testUploadDropbox();
    void testDownloadDropbox();
    void testRemotePath();
    void testLocalPath();
    void testTimeMap();
};

FsTest::FsTest()
{
}

void FsTest::testCase1()
{
    QVERIFY2(true, "Failure");
}


void FsTest::testDropbox()
{

    FvDropbox dr(NULL);
    if (dr.FvDropboxTryConnect()<1){
        printf("Press ENTER after you authorized the application!\n");
        scanf("%c");

        dr.FvDropboxFinishConnecting();

    }
    QVERIFY2(true, "Failure");
}

void FsTest::testUploadDropbox()
{

    FvDropbox dr(NULL);
    if (dr.FvDropboxTryConnect()<1){
        printf("Press ENTER after you authorized the application!\n");
        scanf("%c");

        dr.FvDropboxFinishConnecting();


    }
    QTemporaryFile tempFile;
    tempFile.open();
    tempFile.putChar('A');
    tempFile.close();
    QString remotePath("/sandbox/tempFile");
    dr.uploadFile(tempFile, remotePath);
    QVERIFY2(true, "Failure");
}

void FsTest::testDownloadDropbox()
{

    FvDropbox dr(NULL);
    if (dr.FvDropboxTryConnect()<1){
        printf("Press ENTER after you authorized the application!\n");
        scanf("%c");

        dr.FvDropboxFinishConnecting();


    }
    QTemporaryFile tempFile;
    tempFile.open();
    tempFile.putChar('A');
    tempFile.close();
    QString remotePath("/sandbox/tempFile");
    dr.uploadFile(tempFile, remotePath);
    tempFile.remove();
    dr.downloadFile(remotePath, tempFile);
    tempFile.open();
    QByteArray data;
    data= tempFile.readAll();
    QVERIFY2(data[0] == 'A', "Failure");
}

void FsTest::testRemotePath()
{
    printf("\nabsolute remote path=%s\n", (FvDropbox::getAbsoluteRemotePath("test")).toStdString().c_str());
    QVERIFY2(FvDropbox::getAbsoluteRemotePath("test") == QString(DROPBOX_PATH_PREFIX "test"), "Failure");
    QString absolutePath = FvDropbox::getAbsoluteRemotePath("test");
    printf("%s", absolutePath.toStdString().c_str());
    QVERIFY2(FvDropbox::getRelativeRemotePath(absolutePath) == QString("test"), "Failure");
}

void FsTest::testLocalPath()
{
    QTemporaryDir tempDir;
    FvFileWatcher fvFile(NULL, tempDir.path());
    printf("\nlocal path: %s\n", fvFile.getAbsolutePath("test").toStdString().c_str());
    //QVERIFY2(FvDropbox::getAbsoluteRemotePath("test") == QString(DROPBOX_PATH_PREFIX "test"), "Failure");
    QString absolutePath = fvFile.getAbsolutePath("test");
    printf("\nfile name=%s\n", fvFile.getRelativePath(absolutePath).toStdString().c_str());
    //QVERIFY2(FvDropbox::getRelativeRemotePath(absolutePath) == QString("test"), "Failure");
}

void FsTest::testTimeMap()
{
    QTemporaryDir tempDir;
    QDir dir(tempDir.path());
    int i;
    for (i=0;i<10;i++){
        QFile tempfile(dir.absoluteFilePath(QString('A'+i)));
        tempfile.open(QFile::WriteOnly);
        tempfile.putChar('A');
        tempfile.close();
    }
    QDir dir2(dir.absoluteFilePath("test"));
    dir2.mkpath(dir2.absolutePath());
    for (i=0;i<10;i++){
        QFile tempfile(dir2.absoluteFilePath(QString('a'+i)));
        tempfile.open(QFile::WriteOnly);
        tempfile.putChar('A');
        tempfile.close();
    }


    FvFileWatcher fvFile(NULL, tempDir.path());
    QMap<QString, QDateTime> map;
    map = fvFile.populateTimeMap();
    for (i=0;i<map.keys().length();i++){
        printf("%s,%s\n",map.keys()[i].toStdString().c_str(), map.values()[i].toString().toStdString().c_str());
    }
}


QTEST_MAIN(FsTest)

#include "tst_fstest.moc"
