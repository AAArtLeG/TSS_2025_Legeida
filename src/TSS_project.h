#pragma once
#include <QtWidgets/QMainWindow>
#include <QtWidgets>
#include <QVector>
#include <vector>
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

    std::vector<int> currentPage;
    int numOfImgs = 1;
    bool isFolderOpenned = false;
    //int prevNumOfImgs = 1;
    

    QVector<DataStorage>* dataBase = nullptr;

    void checkNumOfImg();
    void scanFolderOnce(const QString& dirPath);
    void showPage();
private slots:
    //fileDialogFunctions
    void on_actionOpen_triggered();
    void on_actionSaveAs_triggered();
    void on_actionOpen_folder_triggered();
    void on_comboSelectNumOfImgs_currentIndexChanged();
};
