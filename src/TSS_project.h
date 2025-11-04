#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets>
#include "ui_TSS_project.h"
  


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


private slots:
    //fileDialogFunctions
    void on_actionOpen_triggered();
    void on_actionSaveAs_triggered();
};
