#include "TSS_project.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TSS_project w;
    w.show();
    return a.exec();
}
