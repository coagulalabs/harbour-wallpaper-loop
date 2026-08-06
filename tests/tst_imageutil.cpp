#include <QtTest/QtTest>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include "../src/imageutil.h"

class TstImageUtil : public QObject
{
    Q_OBJECT

private slots:
    void isImageFile_data();
    void isImageFile();

    void clampInterval_data();
    void clampInterval();

    void normalizeOrder_data();
    void normalizeOrder();

    void normalizeScaleMode_data();
    void normalizeScaleMode();

    void nextPreviousWrap();

    void scanFolderFilters();
};

void TstImageUtil::isImageFile_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<bool>("ok");

    QTest::newRow("jpg") << "a.jpg" << true;
    QTest::newRow("jpeg") << "a.JPEG" << true;
    QTest::newRow("png") << "shot.PNG" << true;
    QTest::newRow("webp") << "x.webp" << true;
    QTest::newRow("bmp") << "x.bmp" << true;
    QTest::newRow("gif") << "anim.gif" << true;
    QTest::newRow("mp4") << "clip.mp4" << false;
    QTest::newRow("webm") << "clip.webm" << false;
    QTest::newRow("txt") << "readme.txt" << false;
    QTest::newRow("noext") << "wallpaper" << false;
}

void TstImageUtil::isImageFile()
{
    QFETCH(QString, name);
    QFETCH(bool, ok);
    QCOMPARE(ImageUtil::isImageFile(name), ok);
}

void TstImageUtil::clampInterval_data()
{
    QTest::addColumn<int>("in");
    QTest::addColumn<int>("out");

    QTest::newRow("below") << 1 << 15;
    QTest::newRow("min") << 15 << 15;
    QTest::newRow("defaultish") << 300 << 300;
    QTest::newRow("max") << 86400 << 86400;
    QTest::newRow("above") << 999999 << 86400;
}

void TstImageUtil::clampInterval()
{
    QFETCH(int, in);
    QFETCH(int, out);
    QCOMPARE(ImageUtil::clampIntervalSeconds(in), out);
}

void TstImageUtil::normalizeOrder_data()
{
    QTest::addColumn<QString>("in");
    QTest::addColumn<QString>("out");

    QTest::newRow("seq") << "SEQUENTIAL" << "SEQUENTIAL";
    QTest::newRow("shuffle") << "SHUFFLE" << "SHUFFLE";
    QTest::newRow("junk") << "random" << "SEQUENTIAL";
}

void TstImageUtil::normalizeOrder()
{
    QFETCH(QString, in);
    QFETCH(QString, out);
    QCOMPARE(ImageUtil::normalizeOrder(in), out);
}

void TstImageUtil::normalizeScaleMode_data()
{
    QTest::addColumn<QString>("in");
    QTest::addColumn<QString>("out");

    QTest::newRow("fill") << "FILL" << "FILL";
    QTest::newRow("fit") << "FIT" << "FIT";
    QTest::newRow("contain") << "CONTAIN" << "CONTAIN";
    QTest::newRow("junk") << "stretch" << "FILL";
}

void TstImageUtil::normalizeScaleMode()
{
    QFETCH(QString, in);
    QFETCH(QString, out);
    QCOMPARE(ImageUtil::normalizeScaleMode(in), out);
}

void TstImageUtil::nextPreviousWrap()
{
    QCOMPARE(ImageUtil::nextIndex(0, 0), 0);
    QCOMPARE(ImageUtil::previousIndex(0, 0), 0);
    QCOMPARE(ImageUtil::nextIndex(2, 3), 0);
    QCOMPARE(ImageUtil::nextIndex(0, 3), 1);
    QCOMPARE(ImageUtil::previousIndex(0, 3), 2);
    QCOMPARE(ImageUtil::previousIndex(1, 3), 0);
}

void TstImageUtil::scanFolderFilters()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    const QString root = tmp.path();
    QVERIFY(QFile::copy(QStringLiteral("/etc/hosts"), root + QStringLiteral("/note.txt"))
            || QFile(root + QStringLiteral("/note.txt")).open(QIODevice::WriteOnly));
    {
        QFile f(root + QStringLiteral("/photo.JPG"));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("x");
    }
    {
        QFile f(root + QStringLiteral("/clip.mp4"));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("x");
    }
    {
        QFile f(root + QStringLiteral("/anim.gif"));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("x");
    }

    QDir dir(root);
    const QFileInfoList files = dir.entryInfoList(ImageUtil::imageFilters(), QDir::Files, QDir::Name);
    QStringList kept;
    for (const QFileInfo &info : files) {
        if (ImageUtil::isImageFile(info.fileName()))
            kept << info.fileName().toLower();
    }
    kept.sort();
    QCOMPARE(kept, QStringList() << QStringLiteral("anim.gif") << QStringLiteral("photo.jpg"));
}

QTEST_MAIN(TstImageUtil)
#include "tst_imageutil.moc"
