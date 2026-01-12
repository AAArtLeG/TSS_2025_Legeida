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
#include <QRubberBand>
#include <QScrollBar>
#include <iostream>

TSS_project::TSS_project(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::TSS_projectClass),         
    scene(new QGraphicsScene(this))
{
    ui->setupUi(this);

    ui->graphicsView->setScene(scene);
    currentPage.resize(numOfImgs);

    setupFilterMenu();

    metaData.load();
    std::cout << "Meta JSON path:" << metaData.getMetaStoragePath().toStdString();

    ui->buttonBack->setEnabled(false);

    cropBand = new QRubberBand(QRubberBand::Rectangle, ui->graphicsView->viewport());
    cropBand->hide();

    //to make eventFilter() get called, we must set which object we will be filtred
    //I choose to install viewport as an event filter, because on this inner widget of graphicsView technically arrived all mouse events
    ui->graphicsView->viewport()->installEventFilter(this);

    /*for (const QByteArray& f : QImageReader::supportedImageFormats())
        qDebug() << f;*/
}

TSS_project::~TSS_project()
{
    delete ui;
}

void TSS_project::setupFilterMenu() {
    auto* menu = new QMenu(this);

    auto* modeGroup = new QActionGroup(this);
    modeGroup->setExclusive(true); // only one option from QActionGroup can be selected (checked)

    QAction* actNoFilter = menu->addAction("No filter");
    actNoFilter->setCheckable(true);
    actNoFilter->setChecked(true);
    ui->toolButtonFilter->setText("No filter");
    modeGroup->addAction(actNoFilter);

    QAction* actSortByDate = menu->addAction("Sort by date");
    actSortByDate->setCheckable(true);
    modeGroup->addAction(actSortByDate);

    QAction* actSortByRating = menu->addAction("Sort by rating");
    actSortByRating->setCheckable(true);
    modeGroup->addAction(actSortByRating);

    QMenu* tagSubMenu = menu->addMenu("Filter by tag");

    QAction* actTagAnimal = tagSubMenu->addAction("Animal");
    QAction* actTagLandscape = tagSubMenu->addAction("Landscape");

    ui->toolButtonFilter->setMenu(menu);
    ui->toolButtonFilter->setPopupMode(QToolButton::InstantPopup);

    //catching currentIndexChange from all options from action group
    connect(actNoFilter, &QAction::triggered, this, [this] {
        if (!confirmUnsavedChanges())
            return;

        currentSortIdx = 0;
        ui->toolButtonFilter->setText("No filter");
        filterCurrentIndexChanged(0); // бывший индекс combobox
        });

    connect(actSortByDate, &QAction::triggered, this, [this] {
        if (!confirmUnsavedChanges())
            return;

        currentSortIdx = 1;
        ui->toolButtonFilter->setText("Sort by date");
        filterCurrentIndexChanged(1);
        });

    connect(actSortByRating, &QAction::triggered, this, [this] {
        if (!confirmUnsavedChanges())
        return;

        currentSortIdx = 2;
        ui->toolButtonFilter->setText("Sort by rating");
        filterCurrentIndexChanged(2);
        });

    connect(actTagAnimal, &QAction::triggered, this, [this] {
        if (!confirmUnsavedChanges())
            return;

        currentSortIdx = -1;
        ui->toolButtonFilter->setText("Animal");
        filterCurrentIndexChanged("Animal");
        });

    connect(actTagLandscape, &QAction::triggered, this, [this] {
        if (!confirmUnsavedChanges())
            return;

        currentSortIdx = -2;
        ui->toolButtonFilter->setText("Landscape");
        filterCurrentIndexChanged("Landscape");
        });
}

bool TSS_project::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == ui->graphicsView->viewport() && isCropInProgress && item && !currentImage.isNull()) {

        if (ev->type() == QEvent::MouseButtonPress) {
            //catch Mouse event
            auto* me = static_cast<QMouseEvent*>(ev);
            if (me->button() == Qt::LeftButton) {
                cropStart = me->pos();
                //create rect which start at cropStart with zero size
                cropBand->setGeometry(QRect(cropStart, QSize()));
                cropBand->show();
                //no default processing of this event
                return true;
            }
        }

        if (ev->type() == QEvent::MouseMove) {
            auto* me = static_cast<QMouseEvent*>(ev);
            //if before was left click catched
            if (cropBand->isVisible()) {
                cropBand->setGeometry(QRect(cropStart, me->pos()).normalized());
                return true;
            }
        }

        if (ev->type() == QEvent::MouseButtonRelease) {
            auto* me = static_cast<QMouseEvent*>(ev);
            if (me->button() == Qt::LeftButton && cropBand->isVisible()) {
                cropRectFromView = cropBand->geometry().normalized();
                return true;
            }
        }
    }

    //if its not our cropping case -> default event processing
    return QMainWindow::eventFilter(obj, ev);
}

int TSS_project::findInOriginByPath(const QString& absPath) {
    for (int i = 0; i < originDataBase.size(); ++i) {
        if (originDataBase[i].getImgPath() == absPath)
            return i;
    }
    return -1;
}

void TSS_project::resaveToOrigin() {
    const QString p = currentImgPath;
    const int oi = findInOriginByPath(p);
    if (oi < 0) return;

    originDataBase[oi].setRating(dataBase[currentImgIdx].getRating());
    originDataBase[oi].setTag(dataBase[currentImgIdx].getTag());
}

void TSS_project::clearSelection() {
    currentImage = QImage();
    currentImgPath.clear();
    currentImgIdx = -1;

    ui->comboBoxRating->blockSignals(true);
    ui->comboBoxTag->blockSignals(true);

    ui->comboBoxRating->setCurrentIndex(0);
    ui->comboBoxTag->setCurrentIndex(0);

    ui->comboBoxRating->setEnabled(false);
    ui->comboBoxTag->setEnabled(false);

    ui->comboBoxRating->blockSignals(false);
    ui->comboBoxTag->blockSignals(false);

    ui->pastelBtn->setEnabled(true);
    ui->monochromeBtn->setEnabled(true);
    ui->vintageBtn->setEnabled(true);
    ui->sepiaBtn->setEnabled(true);
    ui->negativeBtn->setEnabled(true);

    ui->pastelBtn->blockSignals(true);
    ui->pastelBtn->setChecked(false);
    ui->pastelBtn->blockSignals(false);

    ui->monochromeBtn->blockSignals(true);
    ui->monochromeBtn->setChecked(false);
    ui->monochromeBtn->blockSignals(false);

    ui->vintageBtn->blockSignals(true);
    ui->vintageBtn->setChecked(false);
    ui->vintageBtn->blockSignals(false);

    ui->sepiaBtn->blockSignals(true);
    ui->sepiaBtn->setChecked(false);
    ui->sepiaBtn->blockSignals(false);

    ui->negativeBtn->blockSignals(true);
    ui->negativeBtn->setChecked(false);
    ui->negativeBtn->blockSignals(false);

    ui->opacityWM->setEnabled(true);
    ui->watermarkBtn->setEnabled(true);
    ui->offWatermarkBtn->setEnabled(true);
}

bool TSS_project::confirmUnsavedChanges() {
    if (isEditing) {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Question);
        box.setWindowTitle(tr("Unsaved changes"));
        box.setText(tr("You have unsaved changes. Save before navigating away?"));

        QPushButton* btnSave = box.addButton(tr("Save"), QMessageBox::AcceptRole);
        QPushButton* btnSaveAs = box.addButton(tr("Save As…"), QMessageBox::ActionRole);
        QPushButton* btnDiscard = box.addButton(tr("Discard"), QMessageBox::DestructiveRole);
        QPushButton* btnCancel = box.addButton(tr("Cancel"), QMessageBox::RejectRole);

        box.setDefaultButton(btnSave);

        box.exec();

        if (box.clickedButton() == btnSave) {
            if (!saveCurrentImage()){
                std::cout << "Error during saving";
                return false;
            }
            
            endCrop();
            ui->buttonBack->setEnabled(false);
            //isEditing = false;
            return true;
        }
        else if (box.clickedButton() == btnSaveAs) {
            if (!saveCurrentImageAs()) {
                std::cout << "Error during saving";
                return false;
            }

            if (isFolderOpenned && !actualDirPath.isEmpty()) {
                originDataBase = DataStorage::scanFolder(actualDirPath, dataBase, metaData);
            }

            //isEditing = false;
            //scanFolderOnce(actualDirPath);

            endCrop();
            ui->buttonBack->setEnabled(false);

            return true;
        }
        else if (box.clickedButton() == btnDiscard) {
            isEditing = false;
            ui->buttonBack->setEnabled(false);

            endCrop();              
            editor.cleanEditor();   

            clearSelection();      

            return true;
        }
        else {
            return false;
        }
    }
    else
        return true;
}

void TSS_project::buildCurrentPage(int firstIdx) {
    currentPage.resize(numOfImgs);

    for (int i = 0; i < numOfImgs; ++i) {
        int idx = firstIdx + i;

        if (idx < dataBase.size())
            currentPage[i] = idx;
        else
            currentPage[i] = -1;
    }
}

void TSS_project::on_actionOpen_triggered()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Images (*.png *.jpg *.jpeg *.bmp *.gif);;All files (*.*)"));
    if (fileName.isEmpty()) return;

    if (!confirmUnsavedChanges())
        return;

    if (isCropInProgress) 
        endCrop();
    editor.endCrop();
    editor.cleanEditor();
    isEditing = false;
    currentImageOrigin = QImage();
    cropBand->hide();
    cropRectFromView = QRect();

    QImage img(fileName);
    if (img.isNull()) return;
    
    currentImage = img;
    currentImgPath = fileName;
    currentImgIdx = -1;
    scene->clear();
    item = scene->addPixmap(QPixmap::fromImage(img));
    item->setTransformationMode(Qt::SmoothTransformation);
    scene->setSceneRect(item->boundingRect());
    ui->graphicsView->fitInView(item, Qt::KeepAspectRatio);
}

void TSS_project::on_actionSaveAs_triggered()
{
    if (!saveCurrentImageAs()) {
        std::cout << "Error during saving";
        return;
    }
}

bool TSS_project::saveCurrentImage() {
    if (currentImage.isNull()) return false;

    if (currentImgPath.isEmpty())
        return false;

    QImageWriter writer(currentImgPath);      // automaticly detected format

    const bool ok = writer.write(currentImage);
    if (!ok) {
        QMessageBox::warning(this, tr("Save failed"), writer.errorString());
        return false;
    }
    isEditing = false;
    currentImageOrigin = currentImage.convertToFormat(QImage::Format_ARGB32);
    return true;
}

bool TSS_project::saveCurrentImageAs() {
    if (currentImage.isNull()) return false;

    if (currentImgPath.isEmpty())
        return false;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Images (*.png *.jpg *.jpeg *.bmp *.gif);;All files (*.*)"));
    if (fileName.isEmpty()) return false;

    if (QFileInfo(fileName).suffix().isEmpty())
        fileName += ".jpeg";

    const bool ok = currentImage.save(fileName);

    if (!ok) {
        QMessageBox::warning(this, tr("Save failed"), tr("Could not save image."));
        return false;
    }
    isEditing = false;
    currentImgPath = fileName;
    currentImageOrigin = currentImage.convertToFormat(QImage::Format_ARGB32);
    return true;
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

void TSS_project::displayImage(const QImage& img) {
    scene->clear();
    item = nullptr;

    if (img.isNull()) 
        return;

    item = scene->addPixmap(QPixmap::fromImage(img));
    item->setTransformationMode(Qt::SmoothTransformation);
    scene->setSceneRect(item->boundingRect());
    ui->graphicsView->fitInView(item, Qt::KeepAspectRatio);
}

void TSS_project::showImg(const QString& imgPath, const QString& imgName, const int& imgIdx) {
    if (imgPath != currentImgPath) {
        if (isCropInProgress) 
            endCrop();   
        
        editor.endCrop();                  
        editor.cleanEditor();             

        isEditing = false;
        currentImageOrigin = QImage();

        cropBand->hide();
        cropRectFromView = QRect();
    }

    QImage img(imgPath);
    if (!img.isNull()) {
        currentImage = img;
        currentImgPath = imgPath;
        currentImgIdx = imgIdx;

        displayImage(img);

        ui->comboBoxRating->blockSignals(true);
        ui->comboBoxRating->setCurrentIndex(dataBase[currentImgIdx].getRating());
        ui->comboBoxRating->blockSignals(false);

        ui->comboBoxTag->blockSignals(true);
        if (dataBase[currentImgIdx].getTag() == "") {
            ui->comboBoxTag->setCurrentIndex(0);
        }
        if (dataBase[currentImgIdx].getTag() == "Animal") {
            ui->comboBoxTag->setCurrentIndex(1);
        }
        if (dataBase[currentImgIdx].getTag() == "Landscape") {
            ui->comboBoxTag->setCurrentIndex(2);
        }
        ui->comboBoxTag->blockSignals(false);
    }
}

void TSS_project::showPage() {
    scene->clear();
    item = nullptr;
    ui->buttonBack->setEnabled(false);

    if (dataBase.size() == 0)
        return;

    if (numOfImgs == 1) {
        int idx = currentPage[0];
        if (idx < 0 || idx >= dataBase.size())
            return;
        ui->comboBoxRating->setEnabled(true);
        ui->comboBoxTag->setEnabled(true);
        showImg(dataBase[idx].getImgPath(), dataBase[idx].getImgName(), idx);
        return;
    }

    //gallery must be drawn with a neutral (1:1) view transform.
    //I do fitInView(item, Qt::KeepAspectRatio) in displayImg
    //and this scale apllied on everything which QGraphicsView containes, so the gallery grid could not fit into the viewport
    //right now this scale is turned back to 1:1, so our calculations applied "in raw" and every things displayed correctly
    ui->graphicsView->resetTransform();
    ui->graphicsView->horizontalScrollBar()->setValue(0);
    ui->graphicsView->verticalScrollBar()->setValue(0);

    const int gViewW = ui->graphicsView->viewport()->width();
    const int gViewH = ui->graphicsView->viewport()->height();

    // grid parameters
    int cols = 0;
    int rows = 0;
    int thumbW = 0;
    int thumbH = 0;
    const int wGap = 16, hGap = 20;
    const int marginFromTopAndBot = 5;
    const int marginFromLeftAndRight = 5;
    const int labelSize = 36;

    int avaibleH;

    if (numOfImgs == 2 || numOfImgs == 4) {
        cols = 2;
        if (numOfImgs == 2) {
            rows = 1;
            avaibleH = gViewH - 2 * marginFromTopAndBot - (rows - 1) * hGap - rows * labelSize;
            thumbH = avaibleH / rows;
        }
        if (numOfImgs == 4) {
            rows = 2;
            avaibleH = gViewH - 2 * marginFromTopAndBot - (rows - 1) * hGap - rows * labelSize;
            thumbH = avaibleH / rows;
        }
        thumbW = (gViewW - 3 * wGap - 2 * marginFromLeftAndRight) / cols;
    }

    if (numOfImgs == 8) {
        cols = 4;
        rows = 2;
        avaibleH = gViewH - 2 * marginFromTopAndBot - (rows - 1) * hGap - rows * labelSize;
        thumbH = avaibleH / rows;
        thumbW = (gViewW - 5 * wGap - 2 * marginFromLeftAndRight) / cols;
    }

    
    const QFont labelFont("Segoe UI", 9);

    const int count = numOfImgs;
    if (count == 0) return;

    int maxX = 0, maxY = 0;

    for (int i = 0; i < count; ++i) {
        const int r = i / cols;
        const int c = i % cols;

        // top left corner coor
        const int cellX = marginFromLeftAndRight + c * (thumbW + wGap);
        const int cellY = marginFromTopAndBot + r * (thumbH + labelSize + hGap); // +36 for img name

        if (currentPage[i] < 0 || currentPage[i] >= dataBase.size())
            continue;

        const QString path = dataBase[currentPage[i]].getImgPath();
        const QString name = dataBase[currentPage[i]].getImgName();
        const int idx = currentPage[i];
        QImageReader reader(path);

        QSize target(thumbW, thumbH);
        reader.setScaledSize(target);

        QImage thumb = reader.read();
        if (thumb.isNull()) {
            std::cout << "thumb on iter " << i << " is null" << std::endl;
        }

        QPixmap pm = QPixmap::fromImage(thumb);
        pm = pm.scaled(thumbW, thumbH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        auto* pix = new ClickableImgs(pm);
        scene->addItem(pix); // teraz na scene pridavame ne proste QPixmap, a elem ClickableImgs

        // centered
        const int px = cellX + (thumbW - pm.width()) / 2;
        const int py = cellY + (thumbH - pm.height()) / 2;
        pix->setPos(px, py);
        pix->setTransformationMode(Qt::SmoothTransformation);
        pix->setFlag(QGraphicsItem::ItemIsSelectable, true);
        pix->setData(Qt::UserRole, path);
        pix->setData(Qt::UserRole + 1, name);

        connect(pix,  &ClickableImgs::doubleClicked, this, // связываю дабл клик и ловлю по какой конкретно картинке он был
            [this, path, name, idx](QGraphicsPixmapItem* p) {
                QTimer::singleShot(0, this, [this, path, name, idx] {
                    ui->comboBoxRating->setEnabled(true);
                    ui->comboBoxTag->setEnabled(true);
                    ui->buttonBack->setEnabled(true);
                    showImg(path, name, idx);   // call func showImg AFTER prev reading imgPath
                });
            });

        auto* label = scene->addSimpleText(dataBase[currentPage[i]].getImgName(), labelFont);
        label->setBrush(QColor(60, 60, 60));
        label->setPos(cellX, cellY + thumbH + 6);

        maxX = std::max(maxX, cellX + thumbW);
        maxY = std::max(maxY, cellY + thumbH + 24);
    }

    scene->setSceneRect(0, 0, maxX + marginFromLeftAndRight, maxY + marginFromTopAndBot);
}

void TSS_project::on_comboSelectNumOfImgs_currentIndexChanged() {
    int prevNumOfImg = numOfImgs;
    checkNumOfImg();

    if (isEditing) {
        if (numOfImgs != 1) {
            if (!confirmUnsavedChanges()) {
                ui->comboSelectNumOfImgs->blockSignals(true);
                if (prevNumOfImg == 1) {
                    ui->comboSelectNumOfImgs->setCurrentIndex(0);
                }
                if (prevNumOfImg == 2) {
                    ui->comboSelectNumOfImgs->setCurrentIndex(1);
                }
                if (prevNumOfImg == 4) {
                    ui->comboSelectNumOfImgs->setCurrentIndex(2);
                }
                if (prevNumOfImg == 8) {
                    ui->comboSelectNumOfImgs->setCurrentIndex(3);
                }
                ui->comboSelectNumOfImgs->blockSignals(false);
                numOfImgs = prevNumOfImg;
                return;
            }
        }
        else {
            if (isFolderOpenned) {
                currentPage.resize(numOfImgs);

                if (currentImgIdx >= 0)
                    currentPage[0] = currentImgIdx;
                else
                    currentPage[0] = currentPage[0];
                
                ui->buttonBack->setEnabled(false); 

                return;
            }
        }
    }

    if (isFolderOpenned) {
        int currentFirstIndex = currentPage[0];

        if (numOfImgs == 1 && currentImgIdx >= 0) {
            currentFirstIndex = currentImgIdx;  
        }

        currentPage.resize(numOfImgs);
        for (int i = 0; i < numOfImgs; i++) {
            currentPage[i] = i + currentFirstIndex;
            //std::cout << "currentPage idx: " << i << "currentPage[idx] val: " << std::endl;
        }

        if (numOfImgs != 1)
            clearSelection();

        showPage();
    }

}

void TSS_project::on_comboBoxRating_currentIndexChanged(){
    if (currentImage.isNull()) return;

    if (currentImgIdx < 0)
        return;

    if (ui->comboBoxRating->currentIndex() == 0){
        dataBase[currentImgIdx].setRating(0);
        metaData.setInRecords(currentImgPath, 0, dataBase[currentImgIdx].getTag());
        metaData.save();
    }
        

    if (ui->comboBoxRating->currentIndex() == 1){
        dataBase[currentImgIdx].setRating(1);
        metaData.setInRecords(currentImgPath, 1, dataBase[currentImgIdx].getTag());
        metaData.save();
    }
        

    if (ui->comboBoxRating->currentIndex() == 2){
        dataBase[currentImgIdx].setRating(2);
        metaData.setInRecords(currentImgPath, 2, dataBase[currentImgIdx].getTag());
        metaData.save();
    }
        

    if (ui->comboBoxRating->currentIndex() == 3){
        dataBase[currentImgIdx].setRating(3);
        metaData.setInRecords(currentImgPath, 3, dataBase[currentImgIdx].getTag());
        metaData.save();
    }

    if (ui->comboBoxRating->currentIndex() == 4) {
        dataBase[currentImgIdx].setRating(4);
        metaData.setInRecords(currentImgPath, 4, dataBase[currentImgIdx].getTag());
        metaData.save();
    }

    if (ui->comboBoxRating->currentIndex() == 5) {
        dataBase[currentImgIdx].setRating(5);
        metaData.setInRecords(currentImgPath, 5, dataBase[currentImgIdx].getTag());
        metaData.save();
    }

    resaveToOrigin();
        
    if (currentSortIdx == 2) {
        dataBase = originDataBase;
        dataBase = DataStorage::mergeSort(dataBase, "rating");

        int newIdx = -1;
        for (int i = 0; i < dataBase.size(); ++i) {
            if (dataBase[i].getImgPath() == currentImgPath)
                newIdx = i;
        }

        if (newIdx >= 0) {
            currentImgIdx = newIdx;

            if (numOfImgs == 1) {
                currentPage.resize(1);
                currentPage[0] = newIdx;
            }
            else {
                int newStart = (newIdx / numOfImgs) * numOfImgs;
                buildCurrentPage(newStart);
            }

        }
        else {
            clearSelection();
            showPage();
            return;
        }

        if (isEditing)
            return;

        clearSelection();
        showPage();

    }
}

void TSS_project::on_comboBoxTag_currentIndexChanged() {
    if (currentImage.isNull()) return;

    if (currentImgIdx < 0 || currentImgIdx >= dataBase.size())
        return;
        
    QString pathOfImg = currentImgPath;

    QString oldTag = dataBase[currentImgIdx].getTag();

    QString tag = "";

    if (ui->comboBoxTag->currentIndex() == 0) 
        tag = "";
    if (ui->comboBoxTag->currentIndex() == 1) 
        tag = "Animal";
    if (ui->comboBoxTag->currentIndex() == 2) 
        tag = "Landscape";

    if (tag == oldTag) 
        return;

    QString activeFilter;

    if (currentSortIdx == -1)
        activeFilter = "Animal";
    if (currentSortIdx == -2)
        activeFilter = "Landscape";

    bool willDisappearFromCurrentDB = (!activeFilter.isEmpty() && tag != activeFilter);

    if (willDisappearFromCurrentDB && isEditing) {
        if (!confirmUnsavedChanges()) {
            ui->comboBoxTag->blockSignals(true);
            if (oldTag == "Animal")
                ui->comboBoxTag->setCurrentIndex(1);
            else {
                if (oldTag == "Landscape")
                    ui->comboBoxTag->setCurrentIndex(2);
                else
                    ui->comboBoxTag->setCurrentIndex(0);
            }
            
            ui->comboBoxTag->blockSignals(false);
            return;
        }

        // user can choose discardBtn which containes clearSelection() - > no currentIdx and currentImage - > move back tag edit and like all other edits
        if (currentImgIdx < 0 || currentImgIdx >= dataBase.size() || currentImage.isNull()) {
            QString pathToUpdate;
            if (!currentImgPath.isEmpty()) {
                pathToUpdate = currentImgPath;
            }
            else {
                pathToUpdate = pathOfImg;
            }

            int origIdx = findInOriginByPath(pathToUpdate);
            if (origIdx >= 0) {
                originDataBase[origIdx].setTag(tag);
            }

            int ratingForMeta = 0;
            if (origIdx >= 0) 
                ratingForMeta = originDataBase[origIdx].getRating();

            metaData.setInRecords(pathToUpdate, ratingForMeta, tag);
            metaData.save();

            filterCurrentIndexChanged(activeFilter);
            return;
        }
            
    }

   

    dataBase[currentImgIdx].setTag(tag);
    metaData.setInRecords(currentImgPath, dataBase[currentImgIdx].getRating(), tag);
    metaData.save();

    resaveToOrigin();

    if (currentSortIdx == -1) {
        dataBase = originDataBase;
        filterCurrentIndexChanged("Animal");
    }

    if (currentSortIdx == -2) {
        dataBase = originDataBase;
        filterCurrentIndexChanged("Landscape");
    }

}

void TSS_project::filterCurrentIndexChanged(int idx) {
    if (isFolderOpenned) {

        if (idx == 0)
            dataBase = originDataBase;

        if (idx == 1) {
            //TESTS
            /*for (int i = 0; i < 30; ++i)
            std::cout << i << " "
            << dataBase[i].getDateCreated().toString(Qt::ISODate).toStdString()
            << "\n";*/

            dataBase = originDataBase;
            dataBase = DataStorage::mergeSort(dataBase, "date");

            //TESTS
            /*for (int i = 0; i < 30; ++i)
                std::cout << i << " "
                << dataBase[i].getDateCreated().toString(Qt::ISODate).toStdString()
                << "\n";

            std::cout << dataBase.size() - 1 << " "
                << dataBase[dataBase.size() - 1].getImgName().toStdString()
                << "\n";*/

        }

        if (idx == 2){
            dataBase = originDataBase;
            dataBase = DataStorage::mergeSort(dataBase, "rating");
        }

        clearSelection();

        buildCurrentPage(0);

        showPage();
    }
}

void TSS_project::filterCurrentIndexChanged(QString tag) {
    if (!isFolderOpenned) return;

    dataBase = originDataBase;
    dataBase = DataStorage::filterByTag(dataBase, tag);

    clearSelection();

    currentPage.resize(numOfImgs);
    for (int i = 0; i < numOfImgs; ++i) currentPage[i] = i;

    showPage();
}

void TSS_project::on_actionOpen_folder_triggered() {
    const QString dir = QFileDialog::getExistingDirectory(this, tr("Select Folder"));
    if (dir.isEmpty()) return;

    if (!confirmUnsavedChanges())
        return;

    actualDirPath = dir;

    isFolderOpenned = true;
    originDataBase = DataStorage::scanFolder(dir, dataBase, metaData);
    //scanFolderOnce(dir);

    if (currentSortIdx == 1) {
        dataBase = DataStorage::mergeSort(dataBase, "date");
    }

    if (currentSortIdx == 2) {
        dataBase = DataStorage::mergeSort(dataBase, "rating");
    }

    if (currentSortIdx == -1) {
        filterCurrentIndexChanged("Animal");
    }

    if (currentSortIdx == -2) {
        filterCurrentIndexChanged("Landscape");
    }

    checkNumOfImg();
    buildCurrentPage(0);

    clearSelection();

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

void TSS_project::isImgWasReturnToOrigin(const QImage& img) {
    if (isEditing && currentImageOrigin == img)
        isEditing = false;
}

void TSS_project::on_undoCropBtn_clicked() {
    if (!isCropInProgress || currentImage.isNull())
        return;

    QString msg = editor.resetCrops(currentImage);

    if (!msg.isNull() && !msg.isEmpty())
        statusBar()->showMessage(msg, 2000);
    else
        statusBar()->clearMessage();

    cropBand->hide();
    cropRectFromView = QRect();

    // Перерисовать
    if (!currentImage.isNull())
        isImgWasReturnToOrigin(currentImage);
        displayImage(currentImage);
}

void TSS_project::on_applyCropBtn_clicked() {
    if (!isCropInProgress || cropRectFromView.isNull() || currentImage.isNull() || !item)
        return;

    //selected rect to rect in coordinates of scene
    QRectF sceneSel = ui->graphicsView->mapToScene(cropRectFromView).boundingRect();

    QString msg = editor.crop(sceneSel, currentImage, item);

    if (!msg.isNull()) {
        statusBar()->showMessage(msg, 2000);
    }
    else {
        statusBar()->clearMessage();
    }

    cropBand->hide();
    cropRectFromView = QRect();

    if (!currentImage.isNull()) {
        isImgWasReturnToOrigin(currentImage);
        std::cout << "contrast end" << std::endl;
        displayImage(currentImage);
    }
}

void TSS_project::on_cancelCropBtn_clicked() {
    if (!isCropInProgress) 
        return;

    cropBand->hide();
    cropRectFromView = QRect();
}

void TSS_project::endCrop() {
    editor.endCrop();

    isCropInProgress = false;

    ui->cropBtn->blockSignals(true);
    ui->cropBtn->setChecked(false);
    ui->cropBtn->blockSignals(false);

    cropBand->hide();
    cropRectFromView = QRect();

    ui->applyCropBtn->setEnabled(false);
    ui->cancelCropBtn->setEnabled(false);
    ui->undoCropBtn->setEnabled(false);

    ui->leftRotation->setEnabled(true);
    ui->rightRotation->setEnabled(true);
    ui->minusBrightnessBtn->setEnabled(true);
    ui->plusBrightnessBtn->setEnabled(true);
    ui->minusContrastBtn->setEnabled(true);
    ui->plusContrastBtn->setEnabled(true);
    ui->minusSaturationBtn->setEnabled(true);
    ui->plusSaturationBtn->setEnabled(true);

    ui->pastelBtn->setEnabled(true);
    ui->monochromeBtn->setEnabled(true);
    ui->vintageBtn->setEnabled(true);
    ui->sepiaBtn->setEnabled(true);
    ui->negativeBtn->setEnabled(true);
}

void TSS_project::on_cropBtn_toggled(bool checked) {
    if (currentImage.isNull()) {
        ui->cropBtn->blockSignals(true);
        ui->cropBtn->setChecked(false);
        ui->cropBtn->blockSignals(false);
        return;
    } 

    isCropInProgress = checked;

    if (!isCropInProgress) {
        endCrop();
        return;
    }

    editor.startCrop(currentImage);

    if (isEditing == false) {
        isEditing = true;
        currentImageOrigin = currentImage.convertToFormat(QImage::Format_ARGB32);
    }

    ui->applyCropBtn->setEnabled(true);
    ui->cancelCropBtn->setEnabled(true);
    ui->undoCropBtn->setEnabled(true);

    ui->leftRotation->setEnabled(false);
    ui->rightRotation->setEnabled(false);
    ui->minusBrightnessBtn->setEnabled(false);
    ui->plusBrightnessBtn->setEnabled(false);
    ui->minusContrastBtn->setEnabled(false);
    ui->plusContrastBtn->setEnabled(false);
    ui->minusSaturationBtn->setEnabled(false);
    ui->plusSaturationBtn->setEnabled(false);

    ui->pastelBtn->setEnabled(false);
    ui->monochromeBtn->setEnabled(false);
    ui->vintageBtn->setEnabled(false);
    ui->sepiaBtn->setEnabled(false);
    ui->negativeBtn->setEnabled(false);

    ui->opacityWM->setEnabled(false);
    ui->watermarkBtn->setEnabled(false);
    ui->offWatermarkBtn->setEnabled(false);
}

void TSS_project::on_leftRotation_clicked() {
    if (currentImage.isNull()) return;

    if (isEditing == false) {
        isEditing = true;
        currentImageOrigin = currentImage.convertToFormat(QImage::Format_ARGB32);
    }
        

    QImage src = currentImage.convertToFormat(QImage::Format_ARGB32);

    QImage dst = editor.rotate(true, src);

    isImgWasReturnToOrigin(dst);

    if (!dst.isNull()) {
        currentImage = dst;
        displayImage(currentImage);
    }
}

void TSS_project::on_rightRotation_clicked() {
    if (currentImage.isNull()) return;

    if (isEditing == false) {
        isEditing = true;
        currentImageOrigin = currentImage.convertToFormat(QImage::Format_ARGB32);
    }

    QImage src = currentImage.convertToFormat(QImage::Format_ARGB32);

    QImage dst = editor.rotate(false, src);

    isImgWasReturnToOrigin(dst);

    if (!dst.isNull()) {
        currentImage = dst;
        displayImage(currentImage);
    }
}

void TSS_project::on_minusBrightnessBtn_clicked() {
    if (currentImage.isNull())
        return;

    if (!isEditing) {
        isEditing = true;
        currentImageOrigin = currentImage.convertToFormat(QImage::Format_ARGB32);
    }

    QString msg = editor.changeBrightness(-10, currentImage);

    if (!msg.isNull()) {
        statusBar()->showMessage(msg, 2000);
    }
    else {
        statusBar()->clearMessage();
    }
    
    if (!currentImage.isNull()) {
        isImgWasReturnToOrigin(currentImage);
        displayImage(currentImage);
    }
}

void TSS_project::on_plusBrightnessBtn_clicked() {
    if (currentImage.isNull())
        return;

    if (!isEditing) {
        isEditing = true;
        currentImageOrigin = currentImage.convertToFormat(QImage::Format_ARGB32);
    }

    QString msg = editor.changeBrightness(10, currentImage);

    if (!msg.isNull()) {
        statusBar()->showMessage(msg, 2000);
    }
    else {
        statusBar()->clearMessage();
    }
    
    if (!currentImage.isNull()) {
        isImgWasReturnToOrigin(currentImage);
        displayImage(currentImage);
    }
}

void TSS_project::on_minusContrastBtn_clicked() {
    if (currentImage.isNull())
        return;

    if (!isEditing) {
        isEditing = true;
        currentImageOrigin = currentImage.convertToFormat(QImage::Format_ARGB32);
    }

    QString msg = editor.changeContrast(-10, currentImage);

    if (!msg.isNull()) {
        statusBar()->showMessage(msg, 2000);
    }
    else {
        statusBar()->clearMessage();
    }

    if (!currentImage.isNull()) {
        isImgWasReturnToOrigin(currentImage);
        displayImage(currentImage);
    }
}

void TSS_project::on_plusContrastBtn_clicked() {
    if (currentImage.isNull())
        return;

    if (!isEditing) {
        isEditing = true;
        currentImageOrigin = currentImage.convertToFormat(QImage::Format_ARGB32);
    }

    QString msg = editor.changeContrast(10, currentImage);

    if (!msg.isNull()) {
        statusBar()->showMessage(msg, 2000);
    }
    else {
        statusBar()->clearMessage();
    }

    if (!currentImage.isNull()) {
        isImgWasReturnToOrigin(currentImage);
        std::cout << "contrast end" << std::endl;
        displayImage(currentImage);
    }
}

void TSS_project::on_minusSaturationBtn_clicked() {
    if (currentImage.isNull())
        return;

    if (!isEditing) {
        isEditing = true;
        currentImageOrigin = currentImage.convertToFormat(QImage::Format_ARGB32);
    }

    QString msg = editor.changeSaturation(-30, currentImage);

    if (!msg.isNull()) {
        statusBar()->showMessage(msg, 2000);
    }
    else {
        statusBar()->clearMessage();
    }

    if (!currentImage.isNull()) {
        isImgWasReturnToOrigin(currentImage);
        displayImage(currentImage);
    }
}

void TSS_project::on_plusSaturationBtn_clicked() {
    if (currentImage.isNull())
        return;

    if (!isEditing) {
        isEditing = true;
        currentImageOrigin = currentImage.convertToFormat(QImage::Format_ARGB32);
    }

    QString msg = editor.changeSaturation(30, currentImage);

    if (!msg.isNull()) {
        statusBar()->showMessage(msg, 2000);
    }
    else {
        statusBar()->clearMessage();
    }

    if (!currentImage.isNull()) {
        isImgWasReturnToOrigin(currentImage);
        displayImage(currentImage);
    }
}

void TSS_project::on_monochromeBtn_toggled(bool checked) {
    if (currentImage.isNull()) {
        ui->monochromeBtn->blockSignals(true);
        ui->monochromeBtn->setChecked(false);
        ui->monochromeBtn->blockSignals(false);
        return;
    }

    if (!checked) {
        ui->pastelBtn->setEnabled(true);
        ui->sepiaBtn->setEnabled(true);
        ui->vintageBtn->setEnabled(true);

        editor.cleanStyleEditor(currentImage);

        if (!currentImage.isNull()) {
            isImgWasReturnToOrigin(currentImage);
            displayImage(currentImage);
        }

        return;
    }

    if (isEditing == false) {
        isEditing = true;
        currentImageOrigin = currentImage.convertToFormat(QImage::Format_ARGB32);
    }

    editor.applyMonochromeFilter(currentImage);

    ui->pastelBtn->setEnabled(false);
    ui->sepiaBtn->setEnabled(false);
    ui->vintageBtn->setEnabled(false);

    if (!currentImage.isNull()) {
        isImgWasReturnToOrigin(currentImage);
        displayImage(currentImage);
    }
}

void TSS_project::on_sepiaBtn_toggled(bool checked) {
    if (currentImage.isNull()) {
        ui->sepiaBtn->blockSignals(true);
        ui->sepiaBtn->setChecked(false);
        ui->sepiaBtn->blockSignals(false);
        return;
    }

    if (!checked) {
        ui->pastelBtn->setEnabled(true);
        ui->monochromeBtn->setEnabled(true);
        ui->vintageBtn->setEnabled(true);

        editor.cleanStyleEditor(currentImage);

        if (!currentImage.isNull()) {
            isImgWasReturnToOrigin(currentImage);
            displayImage(currentImage);
        }

        return;
    }

    if (isEditing == false) {
        isEditing = true;
        currentImageOrigin = currentImage.convertToFormat(QImage::Format_ARGB32);
    }

    editor.applySepiaFilter(currentImage);

    ui->pastelBtn->setEnabled(false);
    ui->monochromeBtn->setEnabled(false);
    ui->vintageBtn->setEnabled(false);

    if (!currentImage.isNull()) {
        isImgWasReturnToOrigin(currentImage);
        displayImage(currentImage);
    }
}

void TSS_project::on_pastelBtn_toggled(bool checked) {
    if (currentImage.isNull()) {
        ui->pastelBtn->blockSignals(true);
        ui->pastelBtn->setChecked(false);
        ui->pastelBtn->blockSignals(false);
        return;
    }

    if (!checked) {
        ui->monochromeBtn->setEnabled(true);
        ui->sepiaBtn->setEnabled(true);
        ui->vintageBtn->setEnabled(true);

        editor.cleanStyleEditor(currentImage);

        if (!currentImage.isNull()) {
            isImgWasReturnToOrigin(currentImage);
            displayImage(currentImage);
        }

        return;
    }

    if (isEditing == false) {
        isEditing = true;
        currentImageOrigin = currentImage.convertToFormat(QImage::Format_ARGB32);
    }

    editor.applyPastelFilter(currentImage);

    ui->monochromeBtn->setEnabled(false);
    ui->sepiaBtn->setEnabled(false);
    ui->vintageBtn->setEnabled(false);

    if (!currentImage.isNull()) {
        isImgWasReturnToOrigin(currentImage);
        displayImage(currentImage);
    }
}

void TSS_project::on_vintageBtn_toggled(bool checked) {
    if (currentImage.isNull()) {
        ui->vintageBtn->blockSignals(true);
        ui->vintageBtn->setChecked(false);
        ui->vintageBtn->blockSignals(false);
        return;
    }

    if (!checked) {
        ui->pastelBtn->setEnabled(true);
        ui->sepiaBtn->setEnabled(true);
        ui->monochromeBtn->setEnabled(true);

        editor.cleanStyleEditor(currentImage);

        if (!currentImage.isNull()) {
            isImgWasReturnToOrigin(currentImage);
            displayImage(currentImage);
        }

        return;
    }

    if (isEditing == false) {
        isEditing = true;
        currentImageOrigin = currentImage.convertToFormat(QImage::Format_ARGB32);
    }

    editor.applyVintageFilter(currentImage);

    ui->pastelBtn->setEnabled(false);
    ui->sepiaBtn->setEnabled(false);
    ui->monochromeBtn->setEnabled(false);

    if (!currentImage.isNull()) {
        isImgWasReturnToOrigin(currentImage);
        displayImage(currentImage);
    }
}

void TSS_project::on_negativeBtn_toggled(bool checked) {
    if (currentImage.isNull()) {
        ui->negativeBtn->blockSignals(true);
        ui->negativeBtn->setChecked(false);
        ui->negativeBtn->blockSignals(false);
        return;
    }

    if (!checked) {
        editor.offNegative(currentImage);

        if (!currentImage.isNull()) {
            isImgWasReturnToOrigin(currentImage);
            displayImage(currentImage);
        }

        return;
    }

    if (isEditing == false) {
        isEditing = true;
        currentImageOrigin = currentImage.convertToFormat(QImage::Format_ARGB32);
    }

    editor.applyNegative(currentImage);

    if (!currentImage.isNull()) {
        isImgWasReturnToOrigin(currentImage);
        displayImage(currentImage);
    }
}

bool TSS_project::confirmWatermarkPos() {
    QMessageBox box(this);
    box.setIcon(QMessageBox::Question);
    box.setWindowTitle(tr("Select watermark"));
    box.setText(tr("In which corner set watermark?"));

    QPushButton* btnRBC = box.addButton(tr("Right bottom corner"), QMessageBox::AcceptRole);
    QPushButton* btnLBC = box.addButton(tr("Left bottom corner"), QMessageBox::ActionRole);
    QPushButton* btnLTC = box.addButton(tr("Left top corner"), QMessageBox::ActionRole);
    QPushButton* btnRTC = box.addButton(tr("Right top corner"), QMessageBox::ActionRole);
    QPushButton* btnCancel = box.addButton(tr("Cancel"), QMessageBox::RejectRole);

    box.setDefaultButton(btnRBC);

    box.exec();
    unsigned int pos = 0;
    if (box.clickedButton() == btnRBC) {
        if (!editor.setWatermarkPos(pos))
            return false;

        return true;
    }
    else if (box.clickedButton() == btnLBC) {
        pos = 1;
        if (!editor.setWatermarkPos(pos))
            return false;

        return true;
    }
    else if (box.clickedButton() == btnLTC) {
        pos = 2;
        if (!editor.setWatermarkPos(pos))
            return false;

        return true;
    }
    else if (box.clickedButton() == btnRTC) {
        pos = 3;
        if (!editor.setWatermarkPos(pos))
            return false;

        return true;
    }
    else {
        return false;
    }
}

void TSS_project::on_watermarkBtn_clicked() {
    if (currentImage.isNull())
        return;

    QMessageBox box(this);
    box.setIcon(QMessageBox::Question);
    box.setWindowTitle(tr("Which watermark do you want to apply?"));
    box.setText(tr("Which watermark do you want to apply? Select the new one or apply existed (if WM has already been apllied)?"));

    QPushButton* btnApplySelected = box.addButton(tr("Apply selected"), QMessageBox::AcceptRole);
    QPushButton* btnSelectNewWatermark = box.addButton(tr("Select new"), QMessageBox::ActionRole);
    QPushButton* btnCancel = box.addButton(tr("Cancel"), QMessageBox::RejectRole);

    if (isWatermarkSelected) {
        box.setDefaultButton(btnApplySelected);
    }
    else {
        btnApplySelected->setEnabled(false);
        box.setDefaultButton(btnSelectNewWatermark);
    }
    

    box.exec();

    QString msg = QString();

    if (box.clickedButton() == btnApplySelected) {
        if (!confirmWatermarkPos()) {
            statusBar()->showMessage("Error during setting position of watermark", 2000);
            return;
        }

        if (!isEditing) {
            isEditing = true;
            currentImageOrigin = currentImage.convertToFormat(QImage::Format_ARGB32);
        }

        msg = editor.applyWatermark(currentImage, ui->opacityWM->value());
    }
    else if (box.clickedButton() == btnSelectNewWatermark) {
        QString wmPath = QFileDialog::getOpenFileName(
            this,
            tr("Select watermark image"),
            QString(),
            tr("Images (*.png *.jpg *.jpeg *.bmp *.gif);;All files (*.*)")
        );

        if (wmPath.isEmpty())
            return;

        QImage wm(wmPath);
        if (wm.isNull()) {
            QMessageBox::warning(this, tr("Watermark"), tr("Could not load watermark image."));
            return;
        }

        if (!editor.setWatermark(wm)) {
            statusBar()->showMessage("Error during load the watermark iamge.", 2000);
            return;
        }

        isWatermarkSelected = true;

        if (!confirmWatermarkPos()) {
            statusBar()->showMessage("Error during setting position of watermark", 2000);
            return;
        }
           
        if (!isEditing) {
            isEditing = true;
            currentImageOrigin = currentImage.convertToFormat(QImage::Format_ARGB32);
        }

        msg = editor.applyWatermark(currentImage, ui->opacityWM->value());
    }
    else {
        return;
    }

    if (!msg.isNull()) {
        statusBar()->showMessage(msg, 2000);
    }
    else {
        statusBar()->clearMessage();
    }

    if (!currentImage.isNull()) {
        isImgWasReturnToOrigin(currentImage);
        displayImage(currentImage);
    }
}

void TSS_project::on_opacityWM_valueChanged(double value)
{
    if (!editor.watermarkStatus())
        return;

    QString msg = editor.applyWatermark(currentImage, value);

    if (!msg.isNull()) {
        statusBar()->showMessage(msg, 2000);
    }
    else {
        statusBar()->clearMessage();
    }

    if (!currentImage.isNull()) {
        isImgWasReturnToOrigin(currentImage);
        displayImage(currentImage);
    }
}

void TSS_project::on_offWatermarkBtn_clicked() {
    if (currentImage.isNull())
        return;

    if (!editor.watermarkStatus()) {
        return;
    }

    if (!isEditing) {
        isEditing = true;
        currentImageOrigin = currentImage.convertToFormat(QImage::Format_ARGB32);
    }

    editor.offWatermark(currentImage);

    if (!currentImage.isNull()) {
        isImgWasReturnToOrigin(currentImage);
        displayImage(currentImage);
    }
}

void TSS_project::on_buttonLeftScroll_clicked() {
    if (isFolderOpenned) {

        if (!confirmUnsavedChanges())
            return;
        

        int startIdx = currentPage[0];
        int newStartIdx = startIdx - numOfImgs;

        if (newStartIdx < 0)
            return;

        ui->buttonBack->setEnabled(false);
        editor.cleanEditor();
        currentImage = QImage();
        endCrop();
        buildCurrentPage(newStartIdx);
        clearSelection();
        showPage();
    }
}

void TSS_project::on_buttonRightScroll_clicked() {
    if (isFolderOpenned) {

        if (!confirmUnsavedChanges())
            return;

        int startIdx = currentPage[0];
        int newStartIdx = startIdx + numOfImgs;

        if (newStartIdx >= dataBase.size())
            return;

        ui->buttonBack->setEnabled(false);
        editor.cleanEditor();
        currentImage = QImage();
        endCrop();
        buildCurrentPage(newStartIdx);
        clearSelection();
        showPage();
    }
}

void TSS_project::on_buttonBack_clicked() {
    if (isFolderOpenned) {

        if (!confirmUnsavedChanges())
            return;

        editor.cleanEditor();
        currentImage = QImage();
        endCrop();
        clearSelection();
        showPage();
    }
}