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
    numOfImgs = ui->comboSelectNumOfImgs->currentIndex();
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
    
}

void TSS_project::on_comboSelectNumOfImgs_currentIndexChanged() {
    numOfImgs = ui->comboSelectNumOfImgs->currentIndex();
    if (isFolderOpenned) {
        int currentFirstIndex = currentPage[0];
        currentPage.resize(numOfImgs);
        for (int i = 0; i < numOfImgs; i++) {
            currentPage[i] = i + currentFirstIndex;
        }
        showPage();
    }

}

void TSS_project::on_actionOpen_folder_triggered() {
    const QString dir = QFileDialog::getExistingDirectory(this, tr("Select Folder"));
    if (dir.isEmpty()) return;
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