#include "DataStorage.h"

DataStorage::DataStorage() {};
DataStorage::~DataStorage() {};

void DataStorage::setImgName(QString name) {
	this->imgName = name;
};

void DataStorage::setImgPath(QString path) {
	this->imgPath = path;
};

void DataStorage::setDateCreated(QDateTime date) {
	this->dateCreated = date;
};

const QString& DataStorage::getImgName() const {
	return imgName;
};

const QString& DataStorage::getImgPath() const {
	return imgPath;
};

const QDateTime& DataStorage::getDateCreated() const {
	return dateCreated;
}

QVector<DataStorage> DataStorage::scanFolder(const QString& dirPath, QVector<DataStorage>* dataBase) {
    Q_ASSERT(dataBase);
    dataBase->clear();
    QVector<DataStorage> originDB;
    // for one directory
    /*QDir d(dirPath);
    d.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    d.setNameFilters({ "*.png","*.jpg","*.jpeg","*.bmp","*.gif","*.tif","*.tiff","*.webp" });
    d.setSorting(QDir::Name | QDir::IgnoreCase);

    const QFileInfoList infos = d.entryInfoList();
    dataBase->reserve(infos.size());

    for (const QFileInfo& fi : infos) { //fi metadata from photos
        DataStorage i;
        i.setImgName(fi.fileName());
        i.setImgPath(fi.absoluteFilePath());

        dataBase->push_back(i);
    }
    */

    QDirIterator it(
        dirPath, // root folder
        { "*.png","*.jpg","*.jpeg","*.bmp","*.gif","*.tif","*.tiff","*.webp" },
        QDir::Files | QDir::NoDotAndDotDot,
        QDirIterator::Subdirectories
    );

    while (it.hasNext()) {
        const QString path = it.next();
        const QFileInfo fi(path);

        DataStorage i;
        i.setImgName(fi.fileName());
        i.setImgPath(fi.absoluteFilePath());
        i.setDateCreated(fi.birthTime());

        dataBase->push_back(i);
    }

    originDB = *dataBase;
        

    return originDB;
}