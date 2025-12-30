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

private:
    static QByteArray quickFingerprint(const QString& absPath);

    QString filePath() const;

    QHash<QString, PhotoMeta> byPath;
    QMultiHash<QByteArray, QString> fpToPath;
};

