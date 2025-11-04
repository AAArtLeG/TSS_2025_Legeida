#pragma once
#include <QFileDialog>
#include <QImage>
#include <QPixmap>
#include <QString>
#include <QVector>

class DataStorage
{
public:
	DataStorage();
	~DataStorage();

	QString getImgName();
	QString getImgPath();
	void setImgName(QString name);
	void setImgPath(QString path);
private:
	QString imgPath;
	QString imgName;
};

