#pragma once
#include <QtWidgets/QMainWindow>
#include <QtWidgets>
#include <QVector>
#include <vector>
#include "ui_TSS_project.h"
#include "DataStorage.h"
#include "ClickableImgs.h"
#include "ImageEditor.h"

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
    int currentImgIdx;
    QString actualDirPath;

    std::vector<int> currentPage;
    void buildCurrentPage(int firstIdx);

    int numOfImgs = 1;
    bool isFolderOpenned = false;
    bool isEditing = false;
    bool confirmUnsavedChanges();
    //bool isItFirstEdit = true;
    int currentSortIdx = 0;
    void clearSelection();
    //int prevNumOfImgs = 1;

    bool isCropInProgress = false;
    QRubberBand* cropBand = nullptr;
    QPoint cropStart;
    QRect cropRectFromView;
    void endCrop();

    QVector<DataStorage> dataBase;
    MetaDataStorage metaData;
    QVector<DataStorage> originDataBase;
    ImageEditor editor;

    int findInOriginByPath(const QString& absPath);
    void resaveToOrigin();

    void checkNumOfImg();
    void displayImage(const QImage& img);
    void showPage();
    void showImg(const QString& imgPath, const QString& imgName, const int& imgIdx);
    bool saveCurrentImage();
    bool saveCurrentImageAs();
    void setupFilterMenu();
    void filterCurrentIndexChanged(int idx);
    void filterCurrentIndexChanged(QString tag);
    void isImgWasReturnToOrigin(const QImage& img);

    //override virtual func of QObject eventFilter
    //so right now when event is sent to the watched object -> Qt will call eventFilter ON installed filter object
    //BUT because eventFilter is virtual method which we overrided it WILL call my version of eventFilter
    // it nessecary mouse events and build rect for cropping img
    bool eventFilter(QObject* obj, QEvent* ev) override;
    // IMPORTANT: return value controls event will be handled by my func or by default Qt code
    // return true  -> event is handled here, stop propagation (viewport won't receive it) -> my code will run event on itself
    // return false -> let Qt deliver the event normally to the watched object -> my code wont interapt default event processing
private slots:
    //fileDialogFunctions 
    void on_actionOpen_triggered();
    void on_actionSaveAs_triggered();
    void on_actionOpen_folder_triggered();
    void on_comboSelectNumOfImgs_currentIndexChanged();
    void on_comboBoxRating_currentIndexChanged();
    void on_comboBoxTag_currentIndexChanged();
    void on_undoCropBtn_clicked();
    void on_applyCropBtn_clicked();
    void on_cancelCropBtn_clicked();
    void on_cropBtn_toggled(bool checked);
    void on_leftRotation_clicked();
    void on_rightRotation_clicked();
    void on_minusBrightnessBtn_clicked();
    void on_plusBrightnessBtn_clicked(); 
    void on_minusContrastBtn_clicked();
    void on_plusContrastBtn_clicked();
    void on_minusSaturationBtn_clicked();
    void on_plusSaturationBtn_clicked();
    void on_monochromeBtn_toggled(bool checked);
    void on_sepiaBtn_toggled(bool checked);
    void on_pastelBtn_toggled(bool checked);
    void on_vintageBtn_toggled(bool checked);
    void on_negativeBtn_toggled(bool checked);
    void on_buttonLeftScroll_clicked();
    void on_buttonRightScroll_clicked();
    void on_buttonBack_clicked();
}; 
