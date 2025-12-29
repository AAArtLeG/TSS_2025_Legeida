#pragma once
#include <QtWidgets/QMainWindow>
#include <QtWidgets>
#include <QVector>
#include <vector>
#include "ui_TSS_project.h"
#include "DataStorage.h"
#include "ClickableImgs.h"

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
    QImage currentImage;
    QImage currentImageOrigin;
    QString currentImgPath;
	QString currentImgName;
    QString actualDirPath;

    std::vector<int> currentPage;
    int numOfImgs = 1;
    bool isFolderOpenned = false;
    bool isEditing = false;
    //int prevNumOfImgs = 1;
    

    QVector<DataStorage>* dataBase = nullptr;
    QVector<DataStorage> originDataBase;

    void checkNumOfImg();
    void scanFolderOnce(const QString& dirPath);
    void showPage();
    void showImg(const QString imgPath, const QString imgName);
    void sortByDate();
    bool saveCurrentImage();
    bool saveCurrentImageAs();
private slots:
    //fileDialogFunctions
    void on_actionOpen_triggered();
    void on_actionSaveAs_triggered();
    void on_actionOpen_folder_triggered();
    void on_comboSelectNumOfImgs_currentIndexChanged();
    void on_comboBoxFilter_currentIndexChanged();
    void on_buttonLeftScroll_clicked();
    void on_buttonRightScroll_clicked();
    void on_leftRotation_clicked();
    void on_rightRotation_clicked();
};
