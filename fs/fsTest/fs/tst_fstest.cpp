#include <QString>
#include <QtTest>
#include <../../fvdropbox.h>
#include<../../qtdropbox/qdropbox.h>
#include <cstdio>
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
    printf("%s", (FvDropbox::getAbsoluteRemotePath("test")).toStdString().c_str());
    QVERIFY2(FvDropbox::getAbsoluteRemotePath("test") == QString(DROPBOX_PATH_PREFIX "test"), "Failure");
    QString absolutePath = FvDropbox::getAbsoluteRemotePath("test");
    printf("%s", absolutePath.toStdString().c_str());
    QVERIFY2(FvDropbox::getRelativeRemotePath(absolutePath) == QString("test"), "Failure");
}

QTEST_MAIN(FsTest)

#include "tst_fstest.moc"
