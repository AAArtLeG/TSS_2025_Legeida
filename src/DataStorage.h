#pragma once
#include <QFileDialog>
#include <QImage>
#include <QPixmap>
#include <QString>
#include <QVector>
#include "MetaDataStorage.h"
#include <QDateTime>
#include <QFileInfo>
#include <QDirIterator>
#include <QFile>
#include <QCryptographicHash>

class DataStorage
{
public:
	DataStorage();
	~DataStorage();

	const QString& getImgName() const;
	const QString& getImgPath() const;
	const QDateTime& getDateCreated() const;
	const unsigned int& getRating() const;
	const QString& getTag() const;
	void setImgName(QString name);
	void setImgPath(QString path);
	void setDateCreated(QDateTime date);
	void setRating(unsigned int r);
	void setTag(QString t);

	static QVector<DataStorage> scanFolder(const QString& dirPath, QVector<DataStorage>& dataBase, MetaDataStorage& metaData);
	static QVector<DataStorage> mergeByDates(QVector<DataStorage>& L, QVector<DataStorage>& R);
	static QVector<DataStorage> mergeByRating(QVector<DataStorage>& L, QVector<DataStorage>& R);
	static QVector<DataStorage> mergeSort(QVector<DataStorage>& dataBase, QString typeOfFilter);
	static QVector<DataStorage> filterByTag(QVector<DataStorage>& dataBase, QString& typeOfFilter);
private:
	QString imgPath;
	QString imgName;
	QDateTime dateCreated; 	
	unsigned int rating = 0;
	QString tag;
};