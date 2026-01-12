#include "ImageEditor.h"
#include<iostream>

void ImageEditor::cleanEditor() {
	forColorEditsBase = QImage();
	brightnessСhange = 0;
    contrastСhange = 0;
    saturationСhange = 0;

    styleFilter = 0;
    isNegative = false;

    isWatermarkApplied = false;
    watermarkPos = 0;
}

double ImageEditor::applyGamma(double c, double gamma) {
    double cN = c / 255.0;
    if (cN < 0.0) 
        cN = 0.0;
    if (cN > 1.0) 
        cN = 1.0;
    return std::pow(cN, gamma) * 255.0;
}

QString ImageEditor::applyWatermarkLayer(QImage& src) {
    if (!isWatermarkApplied) 
        return QString();
    
    if (src.isNull()) 
        return "Current image is null";

    if (watermarkBase.isNull()) 
        return "Watermark image is null";

    if (src.format() != QImage::Format_ARGB32)
        src = src.convertToFormat(QImage::Format_ARGB32);

    int w = src.width();
    int h = src.height();
    double totalS = double(w) * double(h);
    if (totalS <= 0.0)
        return "WARNING: Image size is 0, colore dits cant be applied";

    int watermarkW = std::max(1, int(w * 0.25));
    int watermarkH = std::max(1, int(h * 0.25));
    QImage wM = watermarkBase.scaled(watermarkW, watermarkH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (wM.isNull()) 
        return "Failed to scale watermark";

    watermarkW = wM.width();
    watermarkH = wM.height();

    // padding from image borders, cant be smaller that 8 px
    int padding = std::max(8, int(std::min(w, h) * 0.02));

    int x0;
    int y0;
    
    if (watermarkPos == 0) { // right bottom
        x0 = w - watermarkW - padding;
        y0 = h - watermarkH - padding;
    }
    else if (watermarkPos == 1) { // left bottom
        x0 = padding;
        y0 = h - watermarkH - padding;
    }
    else if (watermarkPos == 2) { // left top
        x0 = padding;
        y0 = padding;
    }
    else if (watermarkPos == 3) { // right top
        x0 = w - watermarkW - padding;
        y0 = padding;
    }

    // защита от выхода за границы
    if (x0 < 0) 
        x0 = 0;
    if (y0 < 0) 
        y0 = 0;
    if (x0 + watermarkW > w) 
        x0 = std::max(0, w - watermarkW);
    if (y0 + watermarkH > h) 
        y0 = std::max(0, h - watermarkH);

    // 4) смешивание (wm поверх img)
    for (int y = 0; y < watermarkH; ++y) {
        QRgb* srcRow = reinterpret_cast<QRgb*>(src.scanLine(y0 + y));
        QRgb* wMRow = reinterpret_cast<QRgb*>(wM.scanLine(y));

        for (int x = 0; x < watermarkW; ++x) {
            QRgb wMPix = wMRow[x];

            //if opasity is 0 -> no need to set this pix of watermark on src
            int wMAlpha0 = qAlpha(wMPix);
            if (wMAlpha0 == 0)
                continue;

            // rescaled wM opacity -> firsty transform its base opasity to intreval [0;1], and multiply by the required opacity
            double wMAlphaC = (wMAlpha0 / 255.0) * watermarkOpacity; 
                if (wMAlphaC <= 0.0) continue;

            int srcX = x0 + x;
            QRgb srcPix = srcRow[srcX];

            //linear combination beetween src pix and wM pix
            const int r = (int)std::lround(qRed(wMPix) * wMAlphaC + qRed(srcPix) * (1.0 - wMAlphaC));
            const int g = (int)std::lround(qGreen(wMPix) * wMAlphaC + qGreen(srcPix) * (1.0 - wMAlphaC));
            const int b = (int)std::lround(qBlue(wMPix) * wMAlphaC + qBlue(srcPix) * (1.0 - wMAlphaC));

            // оставляем альфу базового изображения (обычно 255)
            srcRow[srcX] = qRgba(toFitChannelRange(r),
                toFitChannelRange(g),
                toFitChannelRange(b),
                qAlpha(srcPix));
        }
    }

    return QString();
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

            if (styleFilter == 1) {
                //monochrome

                double Ym = 0.299 * r + 0.587 * g + 0.114 * b;

                int gray = toFitChannelRange((int)std::lround(Ym));
                r = gray;
                g = gray;
                b = gray;
            }

            if (styleFilter == 2) {
                //serpia: warm brown+gold filter 

                int rS = 0.393 * r + 0.769 * g + 0.189 * b;
                int gS = 0.349 * r + 0.686 * g + 0.168 * b;
                int bS = 0.272 * r + 0.534 * g + 0.131 * b;

                r = toFitChannelRange((int)std::lround(rS));
                g = toFitChannelRange((int)std::lround(gS));
                b = toFitChannelRange((int)std::lround(bS));
            }

            if (styleFilter == 3) {
                // pastel: softer saturation + slight lift to white + small contrast compression
                double Y = 0.299 * r + 0.587 * g + 0.114 * b;

                double kSatPastel = 0.60;
                double rP= Y + (r - Y) * kSatPastel;
                double gP = Y + (g - Y) * kSatPastel;
                double bP = Y + (b - Y) * kSatPastel;

                // 82% of existet channel value + 18% more of white color
                double kWhite = 0.18; 
                rP = rP * (1.0 - kWhite) + 255.0 * kWhite;
                gP = gP * (1.0 - kWhite) + 255.0 * kWhite;
                bP = bP * (1.0 - kWhite) + 255.0 * kWhite;

                double kC = 0.90; // <1 reduces contrast
                rP = (rP - 128.0) * kC + 128.0;
                gP = (gP - 128.0) * kC + 128.0;
                bP = (bP - 128.0) * kC + 128.0;

                r = toFitChannelRange((int)std::lround(rP));
                g = toFitChannelRange((int)std::lround(gP));
                b = toFitChannelRange((int)std::lround(bP));

            }

            if (styleFilter == 4) {
                //vintage: desatur a bit + warm tint(odtien) + lifted shadows (faded blacks(vyblednutie)) + gentle contrast + do corners darkner, center without changes

                double Y = 0.299 * r + 0.587 * g + 0.114 * b;
                double kSatV = 0.75; 
                double rV = Y + (r - Y) * kSatV;
                double gV = Y + (g - Y) * kSatV;
                double bV = Y + (b - Y) * kSatV;

                rV = rV * 1.05 + 8.0;   
                gV = gV * 1.02 + 4.0;
                bV = bV * 0.93 - 6.0;   

                //when gamma < 1 lifts -> shadows and midtones
                //because we rescaled chan value to [0,1] and than pow(x, gamma) -> chan values which was small (in range from 0 to 255) - shadows -> become lighter
                //but big values (in range from 0 to 255) -> very slightly changed
                double gamma = 0.85; 
                rV = applyGamma(rV, gamma);
                gV = applyGamma(gV, gamma);
                bV = applyGamma(bV, gamma);

                // do corners darkner, center without changes

                //center coor
                double cx = (w - 1) * 0.5;
                double cy = (h - 1) * 0.5;
                //distance normalized around center (0 in center, 1 on borders, 2 in corners)
                double dx = (x - cx) / cx;
                double dy = (y - cy) / cy;
                double d2 = dx * dx + dy * dy;     

                double vig = 1.0 - 0.18 * d2;         
                if (vig < 0.0) vig = 0.0;

                //equal decrease -> no color change -> only brightness
                rV *= vig; gV *= vig; bV *= vig;

                r = toFitChannelRange((int)std::lround(rV));
                g = toFitChannelRange((int)std::lround(gV));
                b = toFitChannelRange((int)std::lround(bV));
            }

            if (isNegative) {
                //negative

                r = 255 - r;
                g = 255 - g;
                b = 255 - b;
            }

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

    //appling watermark
    if (isWatermarkApplied && !watermarkBase.isNull()) {
        QString wmMsg = applyWatermarkLayer(img);
        if (!wmMsg.isNull()) {
            if (!msg.isEmpty()) msg += " | ";
            msg += wmMsg;
        }
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

    applyColorEdits(src);

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

    applyColorEdits(dst);

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

    styleFilter = 1;

    QString msg = applyColorEdits(src);

    return msg;
}

QString ImageEditor::applySepiaFilter(QImage& src) {
    if (forColorEditsBase.isNull()) {
        if (src.isNull())
            return "Invalid download of current image";
        forColorEditsBase = src.convertToFormat(QImage::Format_ARGB32);
        brightnessСhange = 0;
        contrastСhange = 0;
        saturationСhange = 0;
    }

    styleFilter = 2;

    QString msg = applyColorEdits(src);

    return msg;
}

QString ImageEditor::applyPastelFilter(QImage& src) {
    if (forColorEditsBase.isNull()) {
        if (src.isNull())
            return "Invalid download of current image";
        forColorEditsBase = src.convertToFormat(QImage::Format_ARGB32);
        brightnessСhange = 0;
        contrastСhange = 0;
        saturationСhange = 0;
    }

    styleFilter = 3;

    QString msg = applyColorEdits(src);

    return msg;
}

QString ImageEditor::applyVintageFilter(QImage& src) {
    if (forColorEditsBase.isNull()) {
        if (src.isNull())
            return "Invalid download of current image";
        forColorEditsBase = src.convertToFormat(QImage::Format_ARGB32);
        brightnessСhange = 0;
        contrastСhange = 0;
        saturationСhange = 0;
    }

    styleFilter = 4;

    QString msg = applyColorEdits(src);

    return msg;
}

QString ImageEditor::applyNegative(QImage& src) {
    if (forColorEditsBase.isNull()) {
        if (src.isNull())
            return "Invalid download of current image";
        forColorEditsBase = src.convertToFormat(QImage::Format_ARGB32);
        brightnessСhange = 0;
        contrastСhange = 0;
        saturationСhange = 0;
    }

    isNegative = true;

    QString msg = applyColorEdits(src);

    return msg;
}

QString ImageEditor::cleanStyleEditor(QImage& src) {
    if (forColorEditsBase.isNull()) {
        if (src.isNull()) 
            return "Invalid download of current image";
        forColorEditsBase = src.convertToFormat(QImage::Format_ARGB32);
    }
    styleFilter = 0;
    return applyColorEdits(src);
}

QString ImageEditor::offNegative(QImage& src) {
    if (forColorEditsBase.isNull()) {
        if (src.isNull()) return "Invalid download of current image";
        forColorEditsBase = src.convertToFormat(QImage::Format_ARGB32);
    }
    isNegative = false;
    return applyColorEdits(src);
}

bool ImageEditor::watermarkStatus() {
    return isWatermarkApplied;
}

QString ImageEditor::offWatermark(QImage& src) {
    if (isWatermarkApplied) {
        isWatermarkApplied = false;
    }

    if (forColorEditsBase.isNull())
        forColorEditsBase = src.convertToFormat(QImage::Format_ARGB32);

    QString msg = applyColorEdits(src);

    return msg;
}

bool ImageEditor::setWatermark(QImage& watermarkSrc) {
    watermarkBase = watermarkSrc.convertToFormat(QImage::Format_ARGB32);
    if (watermarkBase.isNull())
        return false;
    else
        return true;
}

bool ImageEditor::setWatermarkPos(int pos) {
    watermarkPos = pos;

    if (watermarkPos < 0 || watermarkPos > 3){
        watermarkPos = 0;
        return false;
    }
    else
        return true;
}

QString ImageEditor::applyWatermark(QImage& src, double opacity) {
    if (src.isNull()) 
        return "Current image is null";
    if (watermarkBase.isNull()) 
        return "Watermark image save was unsuccessful";

    if (forColorEditsBase.isNull()) {
        forColorEditsBase = src.convertToFormat(QImage::Format_ARGB32);
        brightnessСhange = 0;
        contrastСhange = 0;
        saturationСhange = 0;

        styleFilter = 0;
        isNegative = false;
    }

    isWatermarkApplied = true;

    watermarkOpacity = opacity;

    QString msg = applyColorEdits(src);

    return msg;
}

