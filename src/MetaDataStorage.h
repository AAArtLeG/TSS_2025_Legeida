#pragma once
#include <QHash>
#include <QMultiHash>
#include <QString>
#include <QByteArray>

struct PhotoMeta {
    unsigned int rating = 0;     
    QString tag;        
    QByteArray fp; //hash fingerPrint
};

class MetaDataStorage
{
public:
    PhotoMeta getFromRecords(const QString& absPath);
    QString getMetaStoragePath();

    void setInRecords(const QString& absPath, int rating, const QString& tag);

    bool save();
    bool load();
private:
    static QByteArray quickFingerprint(const QString& absPath);

    QString pathToJSON();

    QHash<QString, PhotoMeta> pathToMeta;
    QMultiHash<QByteArray, QString> fpToPath;
};

