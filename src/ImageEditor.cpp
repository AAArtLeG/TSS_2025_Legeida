#include "ImageEditor.h"
#include<iostream>

void ImageEditor::cleanEditor() {
	forColorEditsBase = QImage();
	brightnessСhange = 0;
}

QImage ImageEditor::rotateLeft(QImage& src) {
    QImage dst(src.height(), src.width(), QImage::Format_ARGB32); // swap h and w

    int w = src.width();
    int h = src.height();

    Q_ASSERT(dst.width() == h);
    Q_ASSERT(dst.height() == w);

    for (int i = 0; i < h; i++) { // y
        const QRgb* srow = reinterpret_cast<const QRgb*>(src.constScanLine(i)); // i-ty riadok
        for (int j = 0; j < w; j++) { // x
            int newJ = i;
            int newI = w - 1 - j;

            QRgb* drow = reinterpret_cast<QRgb*>(dst.scanLine(newI)); // new row

            drow[newJ] = srow[j];
        }
    }

    return dst;
}

QImage ImageEditor::rotateRight(QImage& src) {
    QImage dst(src.height(), src.width(), QImage::Format_ARGB32); // swap h and w

    int w = src.width();
    int h = src.height();

    Q_ASSERT(dst.width() == h);
    Q_ASSERT(dst.height() == w);

    for (int i = 0; i < h; i++) { // y
        const QRgb* srow = reinterpret_cast<const QRgb*>(src.constScanLine(i)); // i-ty riadok
        for (int j = 0; j < w; j++) { // x
            int newI = j;
            int newJ = h - 1 - i;

            QRgb* drow = reinterpret_cast<QRgb*>(dst.scanLine(newI)); // new row
            drow[newJ] = srow[j];
        }
    }

    return dst;
}

QImage ImageEditor::rotate(bool isLeft, QImage& img) {
    QImage dst;

    if (isLeft) {
        std::cout << "rotation left" << std::endl;
        dst = rotateLeft(img);
        if (!forColorEditsBase.isNull())
            forColorEditsBase = rotateLeft(forColorEditsBase);
    }

    if (!isLeft) {
        std::cout << "rotation right" << std::endl;
        dst = rotateRight(img);
        if (!forColorEditsBase.isNull())
            forColorEditsBase = rotateRight(forColorEditsBase);
    }

    return dst;
}

int ImageEditor::toFitChannelRange(int channelValue) {
    if (channelValue < 0)
        channelValue = 0;

    if (channelValue > 255)
        channelValue = 255;

    return channelValue;
}

bool ImageEditor::isPixOnBorder(const QRgb& p) {
    if ((qRed(p) >= 250 && qGreen(p) >= 250 && qBlue(p) >= 250) || (qRed(p) <= 5 && qGreen(p) <= 5 && qBlue(p) <= 5))
        return true;
    else
        return false;
}

QString ImageEditor::changeBrightness(int delta, QImage& src) {
    if (forColorEditsBase.isNull()) {
        forColorEditsBase = src.convertToFormat(QImage::Format_ARGB32);
        brightnessСhange = 0;
    }

    brightnessСhange += delta;
    if (brightnessСhange > 255)
        brightnessСhange = 255;
    if (brightnessСhange < -255)
        brightnessСhange = -255;

    QImage img = forColorEditsBase.copy();

    //QImage img = currentImage.convertToFormat(QImage::Format_ARGB32);

    int w = img.width();
    int h = img.height();

    double totalS = double(w) * double(h);
    if (totalS <= 0.0)
        return "WARNING: Image size is 0, brightness cant be edited";

    double pixOnBorderInOld = 0.0;
    double pixOnBorderInNew = 0.0;

    for (int y = 0; y < h; ++y) {
        QRgb* row = reinterpret_cast<QRgb*>(img.scanLine(y)); // get one horizontal line of pixels
        for (int x = 0; x < w; ++x) {
            QRgb p = row[x];

            if (isPixOnBorder(p))
                pixOnBorderInOld++;

            int a = qAlpha(p);
            int r = toFitChannelRange(qRed(p) + brightnessСhange);
            int g = toFitChannelRange(qGreen(p) + brightnessСhange);
            int b = toFitChannelRange(qBlue(p) + brightnessСhange);

            row[x] = qRgba(r, g, b, a);

            if (isPixOnBorder(row[x]))
                pixOnBorderInNew++;
        }
    }

    oldClipPix = pixOnBorderInOld / totalS;
    newClipPix = pixOnBorderInNew / totalS;
    dClipPix = newClipPix - oldClipPix;

    QString msg = QString();
    if (dClipPix > 0.05 || newClipPix > 0.7) {
        QString msg1 = QString("Clipping: %1% (Δ %2%)")
            .arg(int(newClipPix * 100.0 + 0.5))
            .arg(int(dClipPix * 100.0 + 0.5));

        msg = "WARNING (too many pixels on \"border\") : " + msg1;
        //statusBar()->showMessage(("WARNING (too many pixels on \"border\") : " + msg), 2000);
    }

    if(!img.isNull())
        src = img;

    return msg;
}