#include "MetaDataStorage.h"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QCryptographicHash>

QString MetaDataStorage::pathToJSON() {
	const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation); //AppDataLocation - miesto(folder) pre chranenie systemovych suborov pre aplikaciu 

	QDir().mkpath(base); // it can create if this folder dont exist

	return QDir(base).filePath("photo_meta.json");
}

QString MetaDataStorage::getMetaStoragePath(){
    return pathToJSON();
}

QByteArray MetaDataStorage::quickFingerprint(const QString& absPath) {
    QFile f(absPath);
    if (!f.open(QIODevice::ReadOnly)) return {};

    QCryptographicHash h(QCryptographicHash::Sha1); // 

    const qint64 size = f.size();
    h.addData(reinterpret_cast<const char*>(&size), sizeof(size));

    const qint64 chunk = 64 * 1024; // chunk = 64 KB

    QByteArray first = f.read(chunk);
    h.addData(first);

    // read only last chunk
    if (size > 2 * chunk) {
        f.seek(size - chunk);
        QByteArray last = f.read(chunk);
        h.addData(last);
    }

    return h.result();
}

 bool MetaDataStorage::load() {
	pathToMeta.clear();
	fpToPath.clear();

	QFile f(pathToJSON());

    if (!f.open(QIODevice::ReadOnly)) {
        if (!f.exists()) 
            return true; // file photo_meta.json pokial proste ne bol vytvoreny => pokial zero saves
        return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());

    // object type in json
    // key = absolute path
    //values = {rating, tag, fp}

    const QJsonObject root = doc.object(); // outload all data key->values from file

    for (auto it = root.begin(); it != root.end(); ++it) {
        const QString absPath = it.key();
        if (!it.value().isObject())
            continue;

        const QJsonObject o = it.value().toObject(); // transform all values from key to objects, like "rating"->"itsValue"

        PhotoMeta meta;
        meta.rating = o.value("rating").toInt(0);
        meta.tag = o.value("tag").toString();

        // decode fp from Base64 to QByteArray
        // we cant same fp in JSON in povodny type QByteArray 
        //SO we must transform QByteArray in Base64(use only standart ASCII symbols) during saving
        const QString fpB64 = o.value("fp").toString();
        meta.fp = QByteArray::fromBase64(fpB64.toLatin1());

        pathToMeta.insert(absPath, meta);

        //POZOR
        // POZRET POTOM ESTE KRAT TO CO DOLE, MOZE BYT IMPOSSIBLE

        // fingerprint -> path (if fp exist)
        if (!meta.fp.isEmpty())
            fpToPath.insert(meta.fp, absPath);
    }
    
    return true;
}

 bool MetaDataStorage::save() {
     QJsonObject root;

     // transform pathToMeta in JSON object to write it to photo_meta.json
     for (auto it = pathToMeta.begin(); it != pathToMeta.end(); ++it) {
         const QString path = it.key();
         const PhotoMeta& meta = it.value();

         QJsonObject o;
         o["rating"] = static_cast<int>(meta.rating);
         o["tag"] = meta.tag;
         o["fp"] = QString::fromLatin1(meta.fp.toBase64());

         root.insert(path, o);

         const QJsonDocument doc(root);

         QFile f(pathToJSON());

         // Truncate = celkom rewrite photo_meta.json
         if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
             return false;

         // Compact — to minimaze size
         f.write(doc.toJson(QJsonDocument::Compact));

         return true;
     }
 }

 PhotoMeta MetaDataStorage::getFromRecords(const QString& absPath) {
     const QString path = QDir::cleanPath(absPath);

     // search by absPath
     auto it = pathToMeta.find(path);
     if (it != pathToMeta.end()) // if find(path) return end() => iterator for key = path wasnt found
         return it.value();

     // try to recover rating and tag, if file was removes/renamed
     const QByteArray fp = quickFingerprint(path);
     if (fp.isEmpty())
         return {}; // (rating=0, tag="")

     const QList<QString> candidates = fpToPath.values(fp); // list of values (QString paths), which key = fp

     // POKIAL, if candidatov != teda pre vsetkych candidatov (rating=0, tag="")  
     if (candidates.size() != 1)
         return {};

     const QString oldPath = candidates.first();

     PhotoMeta meta = pathToMeta.value(oldPath);

     pathToMeta.remove(oldPath);
     fpToPath.remove(fp, oldPath);

     meta.fp = fp;
     pathToMeta.insert(path, meta);
     fpToPath.insert(fp, path);

     return meta;
 }

 void MetaDataStorage::setInRecords(const QString& absPath, int rating, const QString& tag) {
     const QString path = QDir::cleanPath(absPath);

     auto it = pathToMeta.find(path);
     // if exist delete oldFp->path
     // no need to delete path->oldMeta, becuse its QHash, no QMultiHash, so insert automaticly 1) delete path->oldMeta -> 2) create path->newMeta
     if (it != pathToMeta.end()) {
         const QByteArray oldFp = it.value().fp;
         if (!oldFp.isEmpty())
             fpToPath.remove(oldFp, path);
     }

     // if selected default settings, no need to save in photo_meta.json
     if (rating == 0 && tag.trimmed().isEmpty()) {
         pathToMeta.remove(path);
         return;
     }

     PhotoMeta meta;
     meta.rating = rating;
     meta.tag = tag;
     meta.fp = quickFingerprint(path);

     pathToMeta.insert(path, meta); 

     if (!meta.fp.isEmpty())
         fpToPath.insert(meta.fp, path);
 }