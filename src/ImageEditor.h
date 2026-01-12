#pragma once
#include <cmath>
#include <QImage>
#include <QPixmap>
#include <QVector>
#include <QImageReader>
#include <QGraphicsPixmapItem>

class ImageEditor
{
public:
	void cleanEditor();

	void startCrop(const QImage& src);
	QString resetCrops(QImage& src);
	QString crop(const QRectF& rect, QImage& src, QGraphicsPixmapItem* item);
	void endCrop();

	double applyGamma(double c, double gamma);

	QImage rotate(bool isLeft, QImage& img);
	QString changeBrightness(int delta, QImage& src);
	QString changeContrast(int delta, QImage& src);
	QString changeSaturation(int delta, QImage& src);

	QString applyMonochromeFilter(QImage& src);
	QString applySepiaFilter(QImage& src);
	QString applyPastelFilter(QImage& src);
	QString applyVintageFilter(QImage& src);
	
	QString offNegative(QImage& src);
	QString cleanStyleEditor(QImage& src);
	QString applyNegative(QImage& src);

	bool watermarkStatus();
	QString offWatermark(QImage& src);
	bool setWatermark(QImage& watermarkSrc);
	bool setWatermarkPos(int pos);
	QString applyWatermark(QImage& src, double opacity);
private:
	QImage rotateLeft(QImage& src);
	QImage rotateRight(QImage& src);

	QString applyWatermarkLayer(QImage& src);
	QString applyColorEdits(QImage& src);

	int toFitChannelRange(int channelValue);
	bool isPixOnBorder(const QRgb& p);

	QImage forColorEditsBase;
	QImage cropSessionBase;
	
	int brightnessСhange = 0;
	int contrastСhange = 0;
	int saturationСhange = 0;
	double oldClipPix = 0;
	double newClipPix = 0;
	double dClipPix = 0;

	bool isCropInProgress = false;
	QImage baseBeforeCrop;

	unsigned int styleFilter = 0;
	bool isNegative = false;

	QImage watermarkBase;
	bool isWatermarkApplied = false;
	int watermarkPos = 0;
	double watermarkOpacity = 0.35;
};