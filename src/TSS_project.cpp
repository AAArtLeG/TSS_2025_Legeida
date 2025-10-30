#include "TSS_project.h"
#include "ui_TSS_project.h" 
#include <QFileDialog>
#include <QImage>
#include <QPixmap>

TSS_project::TSS_project(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::TSS_projectClass),          // ? ??????? ui!
    scene(new QGraphicsScene(this))
{
    ui->setupUi(this);


    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
}

TSS_project::~TSS_project()
{}


void TSS_project::on_actionOpen_triggered()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Images (*.png *.jpg *.jpeg *.bmp *.gif);;All files (*.*)"));
    if (fileName.isEmpty()) return;

    QImage img(fileName);
    if (img.isNull()) return;

    scene->clear();
    item = scene->addPixmap(QPixmap::fromImage(img));
    item->setTransformationMode(Qt::SmoothTransformation);
    scene->setSceneRect(item->boundingRect());
    ui->graphicsView->fitInView(item, Qt::KeepAspectRatio);
}