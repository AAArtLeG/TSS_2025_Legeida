#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_TSS_project.h"

class TSS_project : public QMainWindow
{
    Q_OBJECT

public:
    TSS_project(QWidget *parent = nullptr);
    ~TSS_project();

private:
    Ui::TSS_projectClass ui;
};
