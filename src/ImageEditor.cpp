#include "ImageEditor.h"
#include<iostream>

void ImageEditor::cleanEditor() {
	forColorEditsBase = QImage();
	brightnessСhange = 0;
    contrastСhange = 0;
    saturationСhange = 0;

    unsigned int styleFilter = 0;
    bool isNegative = false;
}

QString ImageEditor::applyColorEdits(QImage& src) {
    if (forColorEditsBase.isNull())
        return "WARNING: Base image is null; cannot apply color edits";

    QImage img = forColorEditsBase.copy();

    int w = img.width();
    int h = img.height();

    double totalS = double(w) * double(h);
    if (totalS <= 0.0)
        return "WARNING: Image size is 0, colore dits cant be applied";

    double pixOnBorderInOld = 0.0;
    double pixOnBorderInNew = 0.0;

    //contrast correction factor
    double c = (259.0 * (contrastСhange + 255.0)) / (255.0 * (259.0 - contrastСhange));

    //koef of how far we go FROM grayscale version of pixel color
    double kSat = 1.0 + (double)saturationСhange / 255.0; // theoraticaly must be from 0 to 2

    for (int y = 0; y < h; ++y) {
        QRgb* row = reinterpret_cast<QRgb*>(img.scanLine(y)); // get one horizontal line of pixels
        for (int x = 0; x < w; ++x) {
            QRgb p = row[x];

            if (isPixOnBorder(p))
                pixOnBorderInOld++;

            int a = qAlpha(p);

            //overall formula: sturation(contrast(channel)) + brightness
            int r = toFitChannelRange(int(std::lround((qRed(p) - 128.0) * c + 128.0)));
            int g = toFitChannelRange(int(std::lround((qGreen(p) - 128.0) * c + 128.0)));
            int b = toFitChannelRange(int(std::lround((qBlue(p) - 128.0) * c + 128.0)));

            // if we will imagine RGB like 3D space, THAN line R=G=B -> ray from (0,0,0) is grayscale version of color
            // and (Y, Y, Y) is this grayscale version (G.V.) of our pixel
            // saturation adjustment scales the deviation vector (R-Y, G-Y, B-Y) from R=G=B
            // further from the ray R=G=B -> more saturated; closer -> less saturated.
            double Y = 0.299 * r + 0.587 * g + 0.114 * b;

            r = toFitChannelRange((int)std::lround(Y + (r - Y) * kSat));
            g = toFitChannelRange((int)std::lround(Y + (g - Y) * kSat));
            b = toFitChannelRange((int)std::lround(Y + (b - Y) * kSat));

            r = toFitChannelRange(r + brightnessСhange);
            g = toFitChannelRange(g + brightnessСhange);
            b = toFitChannelRange(b + brightnessСhange);

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

    if (!img.isNull())
        src = img;

    return msg;
}

void ImageEditor::startCrop(const QImage& src)
{
    if (src.isNull())
        return;

    if (forColorEditsBase.isNull())
        forColorEditsBase = src.convertToFormat(QImage::Format_ARGB32);

    if (cropSessionBase.isNull())
        cropSessionBase = forColorEditsBase;
}

void ImageEditor::endCrop() {
    cropSessionBase = QImage();
}

QString ImageEditor::resetCrops(QImage& src) {
    if (cropSessionBase.isNull())
        return "Nothing to reset";

    forColorEditsBase = cropSessionBase.copy();

    return applyColorEdits(src);
}

QString ImageEditor::crop(const QRectF& rect, QImage& src, QGraphicsPixmapItem* item) {
    if (!item || src.isNull())
        return "Invalid state for cropping";

    if (forColorEditsBase.isNull())
        forColorEditsBase = src.convertToFormat(QImage::Format_ARGB32);

    if (cropSessionBase.isNull())
        cropSessionBase = forColorEditsBase;

    //rectngle of currentImage in scene
    QRectF sceneImg = item->sceneBoundingRect();

    //crossed part between selected rect and rect of current image
    QRectF sceneCrop = rect.intersected(sceneImg);

    QString msg = QString();
    if (sceneCrop.isEmpty()) {
        msg = "Crop selection is outside the image";
        return msg;
    }

    //transform crosed part to coordinates of item
    QRectF itemCropF = item->mapFromScene(sceneCrop).boundingRect();

    int x = int(std::floor(itemCropF.left()));
    int y = int(std::floor(itemCropF.top()));
    int w = int(std::ceil(itemCropF.width()));
    int h = int(std::ceil(itemCropF.height()));

    QRect cropPx(x, y, w, h);
    cropPx = cropPx.intersected(src.rect());

    if (!cropPx.isValid() || cropPx.isEmpty())
        return "Invalid crop rectangle";

    src = src.copy(cropPx);

    forColorEditsBase = forColorEditsBase.copy(cropPx);

    return msg;
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
        if (src.isNull())
            return "Invalid download of current image";
        forColorEditsBase = src.convertToFormat(QImage::Format_ARGB32);
        brightnessСhange = 0;
        contrastСhange = 0;
        saturationСhange = 0;
    }

    brightnessСhange += delta;
    if (brightnessСhange > 255)
        brightnessСhange = 255;
    if (brightnessСhange < -255)
        brightnessСhange = -255;
    
    QString msg = applyColorEdits(src);

    return msg;
}

QString ImageEditor::changeContrast(int delta, QImage& src) {
    if (forColorEditsBase.isNull()) {
        if (src.isNull())
            return "Invalid download of current image";
        forColorEditsBase = src.convertToFormat(QImage::Format_ARGB32);
        brightnessСhange = 0;
        contrastСhange = 0;
        saturationСhange = 0;
    }

    contrastСhange += delta;
    if (contrastСhange > 255)
        contrastСhange = 255;
    if (contrastСhange < -255)
        contrastСhange = -255;

    QString msg = applyColorEdits(src);

    return msg;
}

QString ImageEditor::changeSaturation(int delta, QImage& src) {
    if (forColorEditsBase.isNull()) {
        if (src.isNull())
            return "Invalid download of current image";
        forColorEditsBase = src.convertToFormat(QImage::Format_ARGB32);
        brightnessСhange = 0;
        contrastСhange = 0;
        saturationСhange = 0;
    }

    saturationСhange += delta;
    if (saturationСhange > 255)
        saturationСhange = 255;
    if (saturationСhange < -255)
        saturationСhange = -255;

    QString msg = applyColorEdits(src);

    return msg;
}

QString ImageEditor::applyMonochromeFilter(QImage& src) {
    if (forColorEditsBase.isNull()) {
        if (src.isNull())
            return "Invalid download of current image";
        forColorEditsBase = src.convertToFormat(QImage::Format_ARGB32);
        brightnessСhange = 0;
        contrastСhange = 0;
        saturationСhange = 0;
    }
}

QString ImageEditor::applySepiaFilter(QImage& src) {

}

QString ImageEditor::applyPastelFilter(QImage& src) {

}

QString ImageEditor::applyVintageFilter(QImage& src) {

}

QString ImageEditor::applyNegative(QImage& src) {

}