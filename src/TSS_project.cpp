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
    currentPage.resize(numOfImgs);

    setupFilterMenu();

    metaData.load();
    std::cout << "Meta JSON path:" << metaData.getMetaStoragePath().toStdString();
}

TSS_project::~TSS_project()
{
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
    QAction* actTagAll = tagSubMenu->addAction("All tags");

    QAction* actTagAnimal = tagSubMenu->addAction("Animal");
    QAction* actTagLandscape = tagSubMenu->addAction("Landscape");

    ui->toolButtonFilter->setMenu(menu);
    ui->toolButtonFilter->setPopupMode(QToolButton::InstantPopup);

    //catching currentIndexChange from all options from action group
    connect(actNoFilter, &QAction::triggered, this, [this] {
        currentSortIdx = 0;
        ui->toolButtonFilter->setText("No filter");
        filterCurrentIndexChanged(0); // бывший индекс combobox
        });

    connect(actSortByDate, &QAction::triggered, this, [this] {
        currentSortIdx = 1;
        ui->toolButtonFilter->setText("Sort by date");
        filterCurrentIndexChanged(1);
        });

    connect(actSortByRating, &QAction::triggered, this, [this] {
        currentSortIdx = 2;
        ui->toolButtonFilter->setText("Sort by rating");
        filterCurrentIndexChanged(2);
        });

    connect(actTagAll, &QAction::triggered, this, [this] {
        //filterCurrentIndexChanged("all"); 
        });

    connect(actTagAnimal, &QAction::triggered, this, [this] {
        //filterCurrentIndexChanged("Animal");
        });

    connect(actTagLandscape, &QAction::triggered, this, [this] {
        //filterCurrentIndexChanged("Landscape");
        });
}

void TSS_project::on_actionOpen_triggered()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Images (*.png *.jpg *.jpeg *.bmp *.gif);;All files (*.*)"));
    if (fileName.isEmpty()) return;

    QImage img(fileName);
    if (img.isNull()) return;
    
    currentImage = img;
    scene->clear();
    item = scene->addPixmap(QPixmap::fromImage(img));
    item->setTransformationMode(Qt::SmoothTransformation);
    scene->setSceneRect(item->boundingRect());
    ui->graphicsView->fitInView(item, Qt::KeepAspectRatio);
}

void TSS_project::on_actionSaveAs_triggered()
{
    if (currentImage.isNull())
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

void TSS_project::showImg(const QString& imgPath, const QString& imgName, const int& imgIdx) {
    QImage img(imgPath);
    if (!img.isNull()) {
        currentImage = img;
        currentImgPath = imgPath;
        currentImgIdx = imgIdx;

        scene->clear();
        item = scene->addPixmap(QPixmap::fromImage(img));
        item->setTransformationMode(Qt::SmoothTransformation);
        scene->setSceneRect(item->boundingRect());
        ui->graphicsView->fitInView(item, Qt::KeepAspectRatio);

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
    if (numOfImgs == 1) {
        const QString& path = dataBase[currentPage[0]].getImgPath();
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

    if (numOfImgs == 8) {
        cols = 4;
        rows = 2;
        thumbW = (gViewW - 5 * wGap) / cols;
        thumbH = (gViewH - 3 * hGap) / rows;
    }

    const int margin = 16;
    const QFont labelFont("Segoe UI", 9);

    const int count = numOfImgs;
    if (count == 0) return;

    int maxX = 0, maxY = 0;

    for (int i = 0; i < count; ++i) {
        const int r = i / cols;
        const int c = i % cols;

        // top left corner coor
        const int cellX = margin + c * (thumbW + wGap);
        const int cellY = margin + r * (thumbH + 36 + hGap); // +36 for img name

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
                    showImg(path, name, idx);   // call func showImg AFTER prev reading imgPath
                });
            });

        auto* label = scene->addSimpleText(dataBase[currentPage[i]].getImgName(), labelFont);
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
            //std::cout << "currentPage idx: " << i << "currentPage[idx] val: " << std::endl;
        }

        showPage();
    }

}

void TSS_project::on_comboBoxRating_currentIndexChanged(){
    if (currentImage.isNull()) return;

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
        
    if (currentSortIdx == 2)
        dataBase = DataStorage::mergeSort(dataBase, "rating");
}

void TSS_project::on_comboBoxTag_currentIndexChanged() {
    if (currentImage.isNull()) return;

    QString tag = "";

    if (ui->comboBoxTag->currentIndex() == 0) {
        dataBase[currentImgIdx].setTag("");
        metaData.setInRecords(currentImgPath, dataBase[currentImgIdx].getRating(), tag);
        metaData.save();
    }

    if (ui->comboBoxTag->currentIndex() == 1) {
        tag = "Animal";
        dataBase[currentImgIdx].setTag("Animal");
        metaData.setInRecords(currentImgPath, dataBase[currentImgIdx].getRating(), tag);
        metaData.save();
    }


    if (ui->comboBoxTag->currentIndex() == 2) {
        tag = "Landscape";
        dataBase[currentImgIdx].setTag("Landscape");
        metaData.setInRecords(currentImgPath, dataBase[currentImgIdx].getRating(), tag);
        metaData.save();
    }
}

void TSS_project::filterCurrentIndexChanged(int idx) {
    if (isFolderOpenned) {
        if (idx == 0)
            DataStorage::scanFolder(actualDirPath, dataBase, metaData);

        if (idx == 1) {
            //TESTS
            /*for (int i = 0; i < 30; ++i)
            std::cout << i << " "
            << dataBase[i].getDateCreated().toString(Qt::ISODate).toStdString()
            << "\n";*/

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

        if (idx == 2)
            dataBase = DataStorage::mergeSort(dataBase, "rating");
        
        showPage();
    }
}

void TSS_project::on_actionOpen_folder_triggered() {
    const QString dir = QFileDialog::getExistingDirectory(this, tr("Select Folder"));
    if (dir.isEmpty()) return;

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
                if (!saveCurrentImage())
                    std::cout << "Error during saving";

            }
            else if (box.clickedButton() == btnSaveAs) {
                if (!saveCurrentImageAs())
                    std::cout << "Error during saving";
                    
                DataStorage::scanFolder(actualDirPath, dataBase, metaData);
                //scanFolderOnce(actualDirPath);
            }
            else if (box.clickedButton() == btnDiscard) {
                currentImage = currentImageOrigin;
                isEditing = false;
            }
            else {
                return;
            }
        }

        if (currentPage[0] - numOfImgs < 0)
            return;

        for (int i = 0; i < numOfImgs; i++)
            currentPage[i] -= numOfImgs;

        showPage();
    }
}

void TSS_project::on_buttonRightScroll_clicked() {
    if (isFolderOpenned) {

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
                if (!saveCurrentImage())
                    std::cout << "Error during saving";
                    
            }
            else if (box.clickedButton() == btnSaveAs) {
                if (!saveCurrentImageAs())
                    std::cout << "Error during saving";

                DataStorage::scanFolder(actualDirPath, dataBase, metaData);
                //scanFolderOnce(actualDirPath);
            }
            else if (box.clickedButton() == btnDiscard) {
                currentImage = currentImageOrigin;
                isEditing = false;
            }
            else {
                return;
            }
        }

        if (currentPage[numOfImgs - 1] + numOfImgs > dataBase.size() - 1)
            return;

        std::cout << "after return" << std::endl;
        for (int i = 0; i < numOfImgs; i++)
            currentPage[i] += numOfImgs;

        showPage();
    }
}

void TSS_project::on_leftRotation_clicked() {
    if (currentImage.isNull()) return;

    if (isEditing == false) {
        isEditing = true;
        currentImageOrigin = currentImage;
    }
        

    QImage src = currentImage.convertToFormat(QImage::Format_ARGB32);

    QImage dst(src.height(), src.width(), QImage::Format_ARGB32); // swap h and w

    int w = src.width();
    int h = src.height();

    Q_ASSERT(dst.width() == h);
    Q_ASSERT(dst.height() == w);

    for (int i = 0; i < h; i++) { // y
        const QRgb* srow = reinterpret_cast<const QRgb*>(src.constScanLine(i)); // i-ty riadok
        for (int j = 0; j < w; j++) { // x
            int newJ = i; 
            int newI = w - 1 - j; 

            QRgb* drow = reinterpret_cast<QRgb*>(dst.scanLine(newI)); // new row

            drow[newJ] = srow[j];
        }
    }

    if (!dst.isNull()) {
        QImage img = dst;
        currentImage = dst;
        scene->clear();
        item = scene->addPixmap(QPixmap::fromImage(img));
        item->setTransformationMode(Qt::SmoothTransformation);
        scene->setSceneRect(item->boundingRect());
        ui->graphicsView->fitInView(item, Qt::KeepAspectRatio);
    }
}

void TSS_project::on_rightRotation_clicked() {
    if (currentImage.isNull()) return;

    if (isEditing == false) {
        isEditing = true;
        currentImageOrigin = currentImage;
    }

    QImage src = currentImage.convertToFormat(QImage::Format_ARGB32);

    QImage dst(src.height(), src.width(), QImage::Format_ARGB32); // swap h and w

    int w = src.width();
    int h = src.height();

    Q_ASSERT(dst.width() == h);
    Q_ASSERT(dst.height() == w);

    for (int i = 0; i < h; i++) { // y
        const QRgb* srow = reinterpret_cast<const QRgb*>(src.constScanLine(i)); // i-ty riadok
        for (int j = 0; j < w; j++) { // x
            int newI = j;
            int newJ = h - 1 - i;

            QRgb* drow = reinterpret_cast<QRgb*>(dst.scanLine(newI)); // new row
            drow[newJ] = srow[j];
        }
    }

    if (!dst.isNull()) {
        QImage img = dst;
        currentImage = dst;
        scene->clear();
        item = scene->addPixmap(QPixmap::fromImage(img));
        item->setTransformationMode(Qt::SmoothTransformation);
        scene->setSceneRect(item->boundingRect());
        ui->graphicsView->fitInView(item, Qt::KeepAspectRatio);
    }
}