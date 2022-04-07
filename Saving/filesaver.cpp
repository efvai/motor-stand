#include "filesaver.h"
#include "ui_filesaver.h"
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

void FileSaver::generateConfig(const QString &path, int ltr11Freq, int ltr22Freq)
{
    QFile config(path + "/" + "config.txt");
    config.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&config);
    out << tr("Параметры модулей:\n");
    out << tr("LTR11:\n");
    out << tr("Частота АЦП: ") << ltr11Freq << tr(" Гц\n");
    out << tr("LTR22:\n");
    out << tr("Частота АЦП: ") << ltr22Freq << tr(" Гц\n");
}
