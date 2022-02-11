#include "mainwindow.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    qRegisterMetaType<std::vector<double>>();
    qRegisterMetaType<QPointF>();
    w.show();
    return a.exec();
}
