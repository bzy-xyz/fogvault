#include <QString>
#include <QtTest>

class FsTest : public QObject
{
    Q_OBJECT

public:
    FsTest();

private Q_SLOTS:
    void testCase1();
};

FsTest::FsTest()
{
}

void FsTest::testCase1()
{
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(FsTest)

#include "tst_fstest.moc"
