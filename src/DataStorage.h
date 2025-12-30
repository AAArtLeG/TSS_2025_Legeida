#pragma once
#include <QFileDialog>
#include <QImage>
#include <QPixmap>
#include <QString>
#include <QVector>
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

	static QVector<DataStorage> scanFolder(const QString& dirPath, QVector<DataStorage>& dataBase);
	static QVector<DataStorage> mergeByDates(QVector<DataStorage>& L, QVector<DataStorage>& R);
	static QVector<DataStorage> mergeSort(QVector<DataStorage>& dataBase);
	static QByteArray quickFingerprint(const QString& absPath);
private:
	QString imgPath;
	QString imgName;
	QDateTime dateCreated; 	
	unsigned int rating;
	QString tag;
};