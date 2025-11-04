#include "DataStorage.h"

DataStorage::DataStorage() {};
DataStorage::~DataStorage() {};

void DataStorage::setImgName(QString name) {
	this->imgName = name;
};

void DataStorage::setImgPath(QString path) {
	this->imgPath = path;
};

QString DataStorage::getImgName() {
	return this->imgName;
};

QString DataStorage::getImgPath() {
	return this->imgPath;
};