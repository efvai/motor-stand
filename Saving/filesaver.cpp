#include "filesaver.h"
#include "ui_filesaver.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

FileSaver::FileSaver(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileSaver)
{

}

FileSaver::~FileSaver()
{
    delete ui;
}

void FileSaver::saveFile(const QString &path, const QString &fileName, const std::vector<float> &data)
{
    QFile outFile(path + "/" + fileName + ".dat");
//    QFile headerFile(":/" + fileName + ".txt");
//    if (!headerFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
//        qDebug() << "Unable to open: " + fileName + ".txt";
//        return;
//    }
//    QTextStream in(&headerFile);
//    QString header;
//    while (!in.atEnd()) {
//        header += in.readLine();
//    }
//    qDebug() << header;
    if (!outFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Unable to open: " + path + fileName + ".dat";
        return;
    }
    QDataStream out(&outFile);
    out.setFloatingPointPrecision(QDataStream::SinglePrecision);
    out.setByteOrder(QDataStream::LittleEndian);
    //out << header;
    for (auto sample : data) {
        out << sample;
    }
}
