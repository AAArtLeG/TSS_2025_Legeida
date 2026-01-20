// REQ-UNIT-FR-2.2-ROTATE-LEFT, REQ-UNIT-FR-2.2-ROTATE-RIGHT

#include <QtTest/QtTest>
#include "ImageEditor.h"

class tst_ImageEditorRotateUnit : public QObject {
    Q_OBJECT
private slots:
    static QImage makeTestImage_2x3();
    void rotateLeft_mapsPixelsCorrectly();   // REQ-UNIT-ROTATE-LEFT
    void rotateRight_mapsPixelsCorrectly();  // REQ-UNIT-ROTATE-RIGHT
};

QImage tst_ImageEditorRotateUnit::makeTestImage_2x3() {
    QImage img(2, 3, QImage::Format_ARGB32);
    img.fill(Qt::transparent);

    img.setPixel(0, 0, qRgb(1, 0, 0));
    img.setPixel(1, 0, qRgb(2, 0, 0));
    img.setPixel(0, 1, qRgb(3, 0, 0));
    img.setPixel(1, 1, qRgb(4, 0, 0));
    img.setPixel(0, 2, qRgb(5, 0, 0));
    img.setPixel(1, 2, qRgb(6, 0, 0));

    return img;
}

void tst_ImageEditorRotateUnit::rotateLeft_mapsPixelsCorrectly() {
    ImageEditor ed;
    QImage src = makeTestImage_2x3();

    QImage dst = ed.rotateLeft(src);

    QCOMPARE(dst.width(), 3);
    QCOMPARE(dst.height(), 2);

    QCOMPARE(qRed(dst.pixel(0, 0)), 2);
    QCOMPARE(qRed(dst.pixel(1, 0)), 4);
    QCOMPARE(qRed(dst.pixel(2, 0)), 6);

    QCOMPARE(qRed(dst.pixel(0, 1)), 1);
    QCOMPARE(qRed(dst.pixel(1, 1)), 3);
    QCOMPARE(qRed(dst.pixel(2, 1)), 5);
}

void tst_ImageEditorRotateUnit::rotateRight_mapsPixelsCorrectly() {
    ImageEditor ed;
    QImage src = makeTestImage_2x3();

    QImage dst = ed.rotateRight(src);

    QCOMPARE(dst.width(), 3);
    QCOMPARE(dst.height(), 2);

    QCOMPARE(qRed(dst.pixel(0, 0)), 5);
    QCOMPARE(qRed(dst.pixel(1, 0)), 3);
    QCOMPARE(qRed(dst.pixel(2, 0)), 1);

    QCOMPARE(qRed(dst.pixel(0, 1)), 6);
    QCOMPARE(qRed(dst.pixel(1, 1)), 4);
    QCOMPARE(qRed(dst.pixel(2, 1)), 2);
}

QTEST_APPLESS_MAIN(tst_ImageEditorRotateUnit)
#include "tst_ImageEditorRotateUnit.moc"