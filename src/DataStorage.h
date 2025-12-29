#pragma once
#include <QFileDialog>
#include <QImage>
#include <QPixmap>
#include <QString>
#include <QVector>
#include <QDateTime>
#include <QFileInfo>
#include <QDirIterator>

class DataStorage
{
public:
	DataStorage();
	~DataStorage();

	const QString& getImgName() const;
	const QString& getImgPath() const;
	const QDateTime& getDateCreated() const;
	const int& getRating() const;
	void setImgName(QString name);
	void setImgPath(QString path);
	void setDateCreated(QDateTime date);
	void setRating(int r);

	static QVector<DataStorage> scanFolder(const QString& dirPath, QVector<DataStorage>& dataBase);
	static QVector<DataStorage> mergeByDates(QVector<DataStorage>& L, QVector<DataStorage>& R);
	static QVector<DataStorage> mergeSort(QVector<DataStorage>& dataBase);
private:
	QString imgPath;
	QString imgName;
	QDateTime dateCreated; 	
	int rating;
};