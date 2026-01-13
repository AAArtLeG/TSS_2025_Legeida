#pragma once
#include <QtWidgets/QMainWindow>
#include <QtWidgets>
#include <QVector>
#include <vector>
#include "ui_TSS_project.h"
#include "DataStorage.h"
#include "ClickableImgs.h"
#include "ImageEditor.h"
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QProgressBar>
#include <utility>

class TSS_project : public QMainWindow
{
    Q_OBJECT

public:
    TSS_project(QWidget *parent = nullptr);
    ~TSS_project();

private:
    Ui::TSS_projectClass *ui = nullptr;

    void blockAllUIForProgressBarDuringSF();
    void unblockAllUIForProgressBarDuringSF();

    // ScanResult = skratka pre log type std::pair<...>
    using ScanResult = std::pair<QVector<DataStorage>, QVector<DataStorage>>;

    void scanFolderWithProgress(const QString& dir, std::function<void(ScanResult&&)> onDone);

    template <typename WorkFn, typename DoneFn>
    void runWithBusyBar(const QString& msg, WorkFn&& work, DoneFn&& done)
    {
        statusBar()->showMessage(msg);
        blockAllUIForProgressBarDuringSF();
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        auto* bar = new QProgressBar(this);
        bar->setTextVisible(false);
        bar->setRange(0, 0); // Indeterminate progress bar – does not show actual progress
        statusBar()->addPermanentWidget(bar);

        // type ResultT = type of function that do hard work
        using ResultT = std::invoke_result_t<WorkFn>;

        auto* watcher = new QFutureWatcher<ResultT>(this);

        // code that i want ot run when fone hard staff ended
        connect(watcher, &QFutureWatcher<ResultT>::finished, this,
            [this, watcher, bar, done = std::forward<DoneFn>(done)]() mutable { // mutable -> this, watcher, bar, done = std::forward<DoneFn>(done) ARE NOT const

                // get result of fone work
                ResultT result = watcher->future().result();

            // delete busy UI
            statusBar()->removeWidget(bar);
            bar->deleteLater();

            unblockAllUIForProgressBarDuringSF();
            statusBar()->clearMessage();

            watcher->deleteLater();

            // pass (call callback for) result of hard staff function 
            done(std::move(result));
            });

        //handle for the result of an asynchronous task -> can catch when hard work finish
        watcher->setFuture(QtConcurrent::run(std::forward<WorkFn>(work)));
    }

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

    bool isWatermarkSelected = false;
    bool confirmWatermarkPos();

    QVector<DataStorage> dataBase;
    MetaDataStorage metaData;
    QVector<DataStorage> originDataBase;
    ImageEditor editor;

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
    void on_actionSave_triggered();
    void on_actionSave_As_2_triggered();
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
    void on_watermarkBtn_clicked();
    void on_opacityWM_valueChanged(double value);
    void on_offWatermarkBtn_clicked();
    void on_buttonLeftScroll_clicked();
    void on_buttonRightScroll_clicked();
    void on_buttonBack_clicked();
    void on_disacrdChangesBtn_clicked(); 
}; 
