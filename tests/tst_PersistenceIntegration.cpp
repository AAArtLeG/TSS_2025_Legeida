// REQ-INTEGRATION-NFR2-PERSISTENCE

#include <QtTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>

#include "MetaDataStorage.h"

class tst_PersistenceIntegration : public QObject
{
    Q_OBJECT
signals:
    void metaChangeRequested(const QString& absPath, int rating, const QString& tag);

private slots:
    void initTestCase();
    void signalSlot_saveLoad_restoresMeta();
};

void tst_PersistenceIntegration::initTestCase()
{
    // so test cant enter real photo_meta.json in AppDataLocation
    QStandardPaths::setTestModeEnabled(true);
    MetaDataStorage m;
    QFile::remove(m.getMetaStoragePath());
}


void tst_PersistenceIntegration::signalSlot_saveLoad_restoresMeta() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString imgPath = dir.filePath("photo1.jpg");
    {
        QFile f(imgPath);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(QByteArray(1024, 'A'));
        f.close();
    }
    QVERIFY(QFileInfo::exists(imgPath));

    MetaDataStorage meta1;
    QVERIFY(meta1.load());
    
    //create spy on signal to verify if emits to signal was
    QSignalSpy reqSpy(this, &tst_PersistenceIntegration::metaChangeRequested);
    QCOMPARE(reqSpy.count(), 0);


    QVERIFY(connect(this, &tst_PersistenceIntegration::metaChangeRequested,
        this, [&](const QString& p, int r, const QString& t) {
            meta1.setInRecords(p, r, t);
            QVERIFY(meta1.save());
        }));

    const int rating = 4;
    const QString tag = "Animal";

    emit metaChangeRequested(imgPath, rating, tag);

    QTRY_VERIFY(reqSpy.count() >= 1);

    // new session
    MetaDataStorage meta2;
    QVERIFY(meta2.load());

    quint64 fileSize = static_cast<quint64>(QFileInfo(imgPath).size());
    PhotoMeta m = meta2.getFromRecords(imgPath, fileSize);

    QCOMPARE((int)m.rating, rating);
    QCOMPARE(m.tag, tag);
}

QTEST_APPLESS_MAIN(tst_PersistenceIntegration)
#include "tst_PersistenceIntegration.moc"