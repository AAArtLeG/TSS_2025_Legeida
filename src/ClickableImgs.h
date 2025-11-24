#pragma once
#include <QFileDialog>
#include <QImage>
#include <QPixmap>
#include <QString>
#include <QVector>
#include <QObject>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>

class ClickableImgs : public QObject, public QGraphicsPixmapItem {
    Q_OBJECT
public:
    using QGraphicsPixmapItem::QGraphicsPixmapItem;   // унаследованные ctors

signals:
    void doubleClicked(QGraphicsPixmapItem* self);

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e) override; // объявление
};