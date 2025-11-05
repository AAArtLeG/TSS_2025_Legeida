#pragma once
#include <QtWidgets/QMainWindow>
#include <QtWidgets>
#include <QVector>
#include "ui_TSS_project.h"
#include "DataStorage.h"

class TSS_project : public QMainWindow
{
    Q_OBJECT

public:
    TSS_project(QWidget *parent = nullptr);
    ~TSS_project();

private:
    Ui::TSS_projectClass *ui = nullptr;

    QGraphicsScene* scene = nullptr;
    QGraphicsPixmapItem* item = nullptr;
    QImage* currentImage;

    QVector<DataStorage>* dataBase = nullptr;

    void scanFolderOnce(const QString& dirPath);
private slots:
    //fileDialogFunctions
    void on_actionOpen_triggered();
    void on_actionSaveAs_triggered();
    void on_actionOpen_folder_triggered();
};
