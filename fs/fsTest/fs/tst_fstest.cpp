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

QTEST_MAIN(FsTest)

#include "tst_fstest.moc"
