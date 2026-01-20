// REQ-GUI-SCROLL tst_TSSProjectScrollThroughImgGUI

// REQ from FinalneZadanie.pdf: 

// Each photo can be displayed in its original resolution after selection from the gallery.If the photo is larger
// than the display window, the user can scroll the window to
// see the entire photo.

#include <QtTest>
#include <QGraphicsView>
#include <QScrollBar>
#include <QWheelEvent>
#include <QCoreApplication>

#include "TSS_project.h"

class tst_TSSProjectScrollThroughImgGUI : public QObject
{
    Q_OBJECT
private slots:
    void bigImage_wheelScroll_changesScrollBar();
};

static void sendWheel(QWidget* viewport, int angleDeltaY)
{
    // wheel scroll is happend under the cursor
    const QPointF pos = viewport->rect().center();
    const QPointF globalPos = viewport->mapToGlobal(pos.toPoint());

    QWheelEvent ev(
        pos,
        globalPos,
        QPoint(0, 0),                 
        QPoint(0, angleDeltaY),       
        Qt::NoButton,
        Qt::NoModifier,
        Qt::ScrollPhase::ScrollUpdate,
        false
    );

    QCoreApplication::sendEvent(viewport, &ev);
}

static QPoint scrollBarArrowClickPoint(QScrollBar* sb, bool rightArrow)
{
    QStyleOptionSlider opt;
    opt.initFrom(sb);
    opt.orientation = sb->orientation();

    const QStyle::SubControl sc = rightArrow ? QStyle::SC_ScrollBarAddLine
        : QStyle::SC_ScrollBarSubLine;

    const QRect r = sb->style()->subControlRect(QStyle::CC_ScrollBar, &opt, sc, sb);
    return r.isValid() ? r.center() : sb->rect().center();
}

void tst_TSSProjectScrollThroughImgGUI::bigImage_wheelScroll_changesScrollBar()
{
    TSS_project w;
    w.show();
    QVERIFY(QTest::qWaitForWindowExposed(&w));

    auto gv = w.findChild<QGraphicsView*>("graphicsView");
    QVERIFY(gv != nullptr);
    QVERIFY(gv->viewport() != nullptr);

    // create a img that in 3 times bigger that vP
    const QSize vp = gv->viewport()->size();
    QVERIFY(vp.width() > 0);
    QVERIFY(vp.height() > 0);

    QImage big(vp.width() * 3, vp.height() * 3, QImage::Format_ARGB32);
    big.fill(Qt::white);
    w.displayImage(big);

    QTest::qWait(5000);

    // is scroll avaible in general
    QVERIFY(gv->verticalScrollBar()->maximum() > 0);
    QVERIFY(gv->horizontalScrollBar()->maximum() > 0);

    // vertical scroll test
    const int before = gv->verticalScrollBar()->value();

    sendWheel(gv->viewport(), -2000);
    QTest::qWait(5000);

    const int after = gv->verticalScrollBar()->value();
    QVERIFY(after != before);
    
    // horizontal scroll
    auto hsb = gv->horizontalScrollBar();

    const int gbefore = hsb->value();

    const QPoint p = scrollBarArrowClickPoint(hsb, true);
    QTest::mouseClick(hsb, Qt::LeftButton, Qt::NoModifier, p);
    QTest::qWait(50);

    const int gafter = hsb->value();
    QVERIFY(gafter != gbefore);

    w.close();
}

QTEST_MAIN(tst_TSSProjectScrollThroughImgGUI)
#include "tst_TSSProjectScrollThroughImgGUI.moc"