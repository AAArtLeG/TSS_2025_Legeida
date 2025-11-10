#include "TSS_project.h"
#include "ui_TSS_project.h" 
#include <QFileDialog>
#include <QImage>
#include <QPixmap>
#include <QVector>
#include <QMessageBox>
#include <QDebug>
#include <QDirIterator>
#include <vector>
#include <QImageReader>
#include <QGraphicsSimpleTextItem>
#include <iostream>

TSS_project::TSS_project(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::TSS_projectClass),         
    scene(new QGraphicsScene(this))
{
    ui->setupUi(this);


    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    currentImage = nullptr;
    dataBase = new QVector<DataStorage>;
    currentPage.resize(numOfImgs);
}

TSS_project::~TSS_project()
{
    delete dataBase;
}

void TSS_project::on_actionOpen_triggered()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Images (*.png *.jpg *.jpeg *.bmp *.gif);;All files (*.*)"));
    if (fileName.isEmpty()) return;

    QImage img(fileName);
    if (img.isNull()) return;
    
    currentImage = &img;
    scene->clear();
    item = scene->addPixmap(QPixmap::fromImage(img));
    item->setTransformationMode(Qt::SmoothTransformation);
    scene->setSceneRect(item->boundingRect());
    ui->graphicsView->fitInView(item, Qt::KeepAspectRatio);
}

void TSS_project::on_actionSaveAs_triggered()
{
    if (currentImage == nullptr)
        return;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Images (*.png *.jpg *.jpeg *.bmp *.gif);;All files (*.*)"));
    if (fileName.isEmpty()) return;

    QImage img(fileName);
    if (img.isNull()) return;

    scene->clear();
    item = scene->addPixmap(QPixmap::fromImage(img));
    item->setTransformationMode(Qt::SmoothTransformation);
    scene->setSceneRect(item->boundingRect());
    ui->graphicsView->fitInView(item, Qt::KeepAspectRatio);
}

void TSS_project::checkNumOfImg() {
    if (ui->comboSelectNumOfImgs->currentIndex() == 0)
        numOfImgs = 1;

    if (ui->comboSelectNumOfImgs->currentIndex() == 1)
        numOfImgs = 2;

    if (ui->comboSelectNumOfImgs->currentIndex() == 2)
        numOfImgs = 4;

    if (ui->comboSelectNumOfImgs->currentIndex() == 3)
        numOfImgs = 8;
}

void TSS_project::scanFolderOnce(const QString& dirPath) {
    Q_ASSERT(dataBase);
    dataBase->clear();

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
        { "*.png","*.jpg","*.jpeg","*.bmp","*.gif","*.tif","*.tiff","*.webp"}, // types of files
        QDir::Files | QDir::NoDotAndDotDot,
        QDirIterator::Subdirectories              
    );

    while (it.hasNext()) {
        const QString path = it.next();
        const QFileInfo fi(path);

        DataStorage i;
        i.setImgName(fi.fileName());
        i.setImgPath(fi.absoluteFilePath());

        dataBase->push_back(i);
    }
}

void TSS_project::showPage() {
    scene->clear();
    if (numOfImgs == 1) {
        const QString& path = dataBase->at(currentPage[0]).getImgPath();
        QImage img(path);
        if (!img.isNull()) {
            scene->clear();
            item = scene->addPixmap(QPixmap::fromImage(img));
            item->setTransformationMode(Qt::SmoothTransformation);
            scene->setSceneRect(item->boundingRect());
            ui->graphicsView->fitInView(item, Qt::KeepAspectRatio);
        }
        return;
    }

    const int gViewW = ui->graphicsView->width();
    const int gViewH = ui->graphicsView->height();

    // grid parameters
    int cols = 0;
    int rows = 0;
    int thumbW = 0;           
    int thumbH = 0;
    const int wGap = 16, hGap = 20;
    if (numOfImgs == 2 || numOfImgs == 4) {
        cols = 2;
        if (numOfImgs == 2) {
            rows = 1;
            thumbH = (gViewH - 2 * hGap) / rows;
        }
        if (numOfImgs == 4) {
            rows = 2;
            thumbH = (gViewH - 3 * hGap) / rows;
        }
        thumbW = (gViewW - 3 * wGap) / cols;
    }

    if (numOfImgs == 8){
        cols = 4;
        rows = 2;
        thumbW = (gViewW - 5 * wGap) / cols;
        thumbH = (gViewH - 3 * hGap) / rows;
    }

        // отступы между ячейками
    const int margin = 16;             // поля по краям
    const QFont labelFont("Segoe UI", 9);

    // Сколько картинок показываем (пример: первые 4)
    const int count = numOfImgs;
    if (count == 0) return;

    int maxX = 0, maxY = 0;

    for (int i = 0; i < count; ++i) {
        const int r = i / cols;
        const int c = i % cols;

        // top left corner coor
        const int cellX = margin + c * (thumbW + wGap);
        const int cellY = margin + r * (thumbH + 36 + hGap); // +36 for img name

        const QString path = dataBase->at(currentPage[i]).getImgPath();
        QImageReader reader(path);
     
        QSize target(thumbW, thumbH);
        reader.setScaledSize(target);

        QImage thumb = reader.read();
        if (thumb.isNull()) {
            std::cout << "thumb on iter " << i << "is null" << std::endl;
        }

        QPixmap pm = QPixmap::fromImage(thumb);
        auto* pix = scene->addPixmap(pm);

        // center
        const int px = cellX + (thumbW - pm.width()) / 2;
        const int py = cellY + (thumbH - pm.height()) / 2;
        pix->setPos(px, py);
        pix->setTransformationMode(Qt::SmoothTransformation);
        pix->setFlag(QGraphicsItem::ItemIsSelectable, true);

        auto* label = scene->addSimpleText(dataBase->at(currentPage[i]).getImgName(), labelFont);
        label->setBrush(QColor(60, 60, 60));
        label->setPos(cellX, cellY + thumbH + 6);

        maxX = std::max(maxX, cellX + thumbW);
        maxY = std::max(maxY, cellY + thumbH + 24);
    }

    scene->setSceneRect(0, 0, maxX + margin, maxY + margin);
}

void TSS_project::on_comboSelectNumOfImgs_currentIndexChanged() {
    checkNumOfImg();
    if (isFolderOpenned) {
        int currentFirstIndex = currentPage[0];
        currentPage.resize(numOfImgs);
        for (int i = 0; i < numOfImgs; i++) {
            currentPage[i] = i + currentFirstIndex;
            std::cout << "currentPage idx: " << i << "currentPage[idx] val: " << std::endl;
        }

        showPage();
    }

}

void TSS_project::on_actionOpen_folder_triggered() {
    const QString dir = QFileDialog::getExistingDirectory(this, tr("Select Folder"));
    if (dir.isEmpty()) return;

    isFolderOpenned = true;
    scanFolderOnce(dir);

    checkNumOfImg();
    currentPage.resize(numOfImgs);
    for (int i = 0; i < numOfImgs; i++) {
        currentPage[i] = i;
    }

    showPage();

    //show first picture
    /*if (!dataBase->isEmpty()) {
        const QString& path = dataBase->at(0).getImgPath();    
        QImage img(path);
        if (!img.isNull()) {
            scene->clear();
            item = scene->addPixmap(QPixmap::fromImage(img));
            item->setTransformationMode(Qt::SmoothTransformation);
            scene->setSceneRect(item->boundingRect());
            ui->graphicsView->fitInView(item, Qt::KeepAspectRatio);
        }
    }*/
}

void TSS_project::on_buttonLeftScroll_clicked() {
    if (isFolderOpenned) {
        if (currentPage[0] - numOfImgs < 0)
            return;

        for (int i = 0; i < numOfImgs; i++)
            currentPage[i] -= numOfImgs;

        showPage();
    }
}

void TSS_project::on_buttonRightScroll_clicked() {
    if (isFolderOpenned) {
        if (currentPage[numOfImgs - 1] + numOfImgs > dataBase->size() - 1)
            return;

        std::cout << "after return" << std::endl;
        for (int i = 0; i < numOfImgs; i++)
            currentPage[i] += numOfImgs;

        showPage();
    }
}