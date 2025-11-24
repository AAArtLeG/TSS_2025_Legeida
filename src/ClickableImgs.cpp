#include "ClickableImgs.h"

void ClickableImgs::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
        emit doubleClicked(this);
        e->accept();
        return;                           // базовый обработчик НЕ вызывается для ЛКМ
    }
    QGraphicsPixmapItem::mouseDoubleClickEvent(e);
}