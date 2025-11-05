#include "DataStorage.h"

DataStorage::DataStorage() {};
DataStorage::~DataStorage() {};

void DataStorage::setImgName(QString name) {
	this->imgName = name;
};

void DataStorage::setImgPath(QString path) {
	this->imgPath = path;
};

const QString& DataStorage::getImgName() const {
	return imgName;
};

const QString& DataStorage::getImgPath() const {
	return imgPath;
};