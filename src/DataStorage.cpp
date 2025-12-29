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

QVector<DataStorage> DataStorage::scanFolder(const QString& dirPath, QVector<DataStorage>& dataBase) {
    dataBase.clear();
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

        dataBase.push_back(i);
    }

    originDB = dataBase;
       
    return originDB;
}

//QVector<DataStorage>* DataStorage::mergeByDates(QVector<DataStorage>& L, QVector<DataStorage>& R) {
//    int i = 0;
//    int j = 0;
//    QVector<DataStorage>* result = new QVector<DataStorage>();
//
//    while (i < L->size() && j < R->size()) {
//        if ((*L)[i].getDateCreated() <= (*R)[j].getDateCreated()) {
//            result->push_back((*L)[i]);
//            i++;
//        }
//        else {
//            result->push_back((*R)[j]);
//            j++;
//        }
//    }
//
//    while (i < L->size()) {
//        result->push_back((*L)[i]);
//        i++;
//    }
//
//    while (j < R->size()) {
//        result->push_back((*R)[j]);
//        j++;
//    }
//
//    return result;
//}
//
//QVector<DataStorage>* DataStorage::mergeSort(QVector<DataStorage>& dataBase) {
//    if (dataBase->size() <= 1)
//        return;
//
//    int mid = dataBase->size() / 2;
//
//    QVector<DataStorage>* L = new QVector<DataStorage>();
//    for (int i = 0; i < mid; i++) {
//        (*L)[i] = (*dataBase)[0];
//    }
//
//    QVector<DataStorage>* R = new QVector<DataStorage>();
//    for (int i = 0; i < (dataBase->size() - mid); i++) {
//        (*R)[i] = (*dataBase)[i+mid];
//    }
//
//    L = mergeSort(L);
//    R = mergeSort(R);
//
//    delete L;
//    delete R;
//
//    return mergeByDates(L, R);
//}