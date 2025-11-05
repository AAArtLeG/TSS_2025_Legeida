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

	const QString& getImgName() const;
	const QString& getImgPath() const;
	void setImgName(QString name);
	void setImgPath(QString path);
private:
	QString imgPath;
	QString imgName;
};