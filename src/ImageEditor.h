#pragma once
#include <cmath>
#include <QImage>
#include <QPixmap>
#include <QVector>
#include <QImageReader>

class ImageEditor
{
public:
	void cleanEditor();
	QImage rotate(bool isLeft, QImage& img);
	QString changeBrightness(int delta, QImage& src);
	QString changeContrast(int delta, QImage& src);
	QString changeSaturation(int delta, QImage& src);
private:
	QImage rotateLeft(QImage& src);
	QImage rotateRight(QImage& src);
	QString applyColorEdits(QImage& src);

	int toFitChannelRange(int channelValue);
	bool isPixOnBorder(const QRgb& p);

	QImage forColorEditsBase;
	
	int brightnessСhange = 0;
	int contrastСhange = 0;
	int saturationСhange = 0;
	double oldClipPix = 0;
	double newClipPix = 0;
	double dClipPix = 0;
};