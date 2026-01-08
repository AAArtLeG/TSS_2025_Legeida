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

void DataStorage::setRating(unsigned int r) {
    this->rating = r;
};

void DataStorage::setTag(QString t) {
    this->tag = t;
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

const unsigned int& DataStorage::getRating() const {
    return rating;
}

const QString& DataStorage::getTag() const {
    return tag;
}

QVector<DataStorage> DataStorage::scanFolder(const QString& dirPath, QVector<DataStorage>& dataBase, MetaDataStorage& metaData) {
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
        //i.rating = 1;

        const QString absPath = fi.absoluteFilePath();
        quint64 size = static_cast<quint64>(fi.size());
        PhotoMeta m = metaData.getFromRecords(absPath, size);
        i.setRating(m.rating);
        i.setTag(m.tag);

        dataBase.push_back(i);
    }

    originDB = dataBase;
       
    return originDB;
}

QVector<DataStorage> DataStorage::mergeByDates(QVector<DataStorage>& L, QVector<DataStorage>& R) {
    int i = 0;
    int j = 0;
    QVector<DataStorage> result;

    while (i < L.size() && j < R.size()) {
        if (L[i].getDateCreated() <= R[j].getDateCreated()) {
            result.push_back(L[i]);
            i++;
        }
        else {
            result.push_back(R[j]);
            j++;
        }
    }

    while (i < L.size()) {
        result.push_back(L[i]);
        i++;
    }

    while (j < R.size()) {
        result.push_back(R[j]);
        j++;
    }

    return result;
}

QVector<DataStorage> DataStorage::mergeByRating(QVector<DataStorage>& L, QVector<DataStorage>& R) {
    int i = 0;
    int j = 0;
    QVector<DataStorage> result;

    while (i < L.size() && j < R.size()) {
        if (L[i].getRating() >= R[j].getRating()) {
            result.push_back(L[i]);
            i++;
        }
        else {
            result.push_back(R[j]);
            j++;
        }
    }

    while (i < L.size()) {
        result.push_back(L[i]);
        i++;
    }

    while (j < R.size()) {
        result.push_back(R[j]);
        j++;
    }

    return result;
}

QVector<DataStorage> DataStorage::mergeSort(QVector<DataStorage>& dataBase, QString typeOfFilter) {
    if (dataBase.size() <= 1)
        return dataBase;

    int mid = dataBase.size() / 2;

    QVector<DataStorage> L;
    L.reserve(mid);
    for (int i = 0; i < mid; i++) {
        L.push_back(dataBase[i]);
    }

    QVector<DataStorage> R;
    R.reserve(dataBase.size() - mid);
    for (int i = 0; i < (dataBase.size() - mid); i++) {
        R.push_back(dataBase[i+mid]);
    }

    if (typeOfFilter == "date") {
        L = mergeSort(L, "date");
        R = mergeSort(R, "date");

        return mergeByDates(L, R);
    }

    if (typeOfFilter == "rating") {
        L = mergeSort(L, "rating");
        R = mergeSort(R, "rating");

        return mergeByRating(L, R);
    }
}

QVector<DataStorage> DataStorage::filterByTag(QVector<DataStorage>& dataBase, QString& typeOfFilter) {
    const QString active = typeOfFilter;
    if (active.isEmpty()) 
        return dataBase;

    for (int i = dataBase.size() - 1; i >= 0; --i) {
        if (dataBase[i].getTag() != active)
            dataBase.removeAt(i);
    }

    return dataBase;
}