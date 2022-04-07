#include "filessaverdialog.h"
#include "Plots/signaldata.h"
#include "filesaver.h"

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QLayout>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QDateTime>


FilesSaverDialog::FilesSaverDialog(QWidget *parent) : QWidget(parent)
{
    setWindowTitle(tr("Сохранение"));
    QLabel *CheckLabel = new QLabel(tr("Выберите модули:"));
    for (int idx = 0; idx < PlotsList::instance().plotsList.size(); idx++) {
        devices.push_back(new QCheckBox(PlotsList::instance().plotsList.at(idx).channelName));
    }
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(CheckLabel);
    QHBoxLayout *checkLayout = new QHBoxLayout;
    for (auto check : qAsConst(devices)) {
        checkLayout->addWidget(check);
    }
    checkLayout->addStretch(1);
    mainLayout->addLayout(checkLayout);
    saveBtn = new QPushButton("Сохранить");
    QPushButton *cancelBtn = new QPushButton("Отмена");
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(cancelBtn);
    btnLayout->addStretch(5);
    filesInfoCheckBox = new QCheckBox(tr("Создать файл с информацией о данных"));
    mainLayout->addWidget(filesInfoCheckBox);
    mainLayout->addLayout(btnLayout);


    setLayout(mainLayout);


    connect(saveBtn, &QPushButton::clicked, this, &FilesSaverDialog::on_save);
    connect(cancelBtn, &QPushButton::clicked, this, &FilesSaverDialog::hide);

}

void FilesSaverDialog::setLtr11Freq(int newLtr11Freq)
{
    m_ltr11Freq = newLtr11Freq;
}

void FilesSaverDialog::setLtr22Freq(int newLtr22Freq)
{
    m_ltr22Freq = newLtr22Freq;
}

void FilesSaverDialog::openSaver()
{
    bool saveActive = false;
    for (int idx = 0; idx < devices.size(); idx++) {
        if (!PlotsList::instance().plotsList.at(idx).enabled) {
            devices.at(idx)->setEnabled(false);
        }
        else {
            devices.at(idx)->setEnabled(true);
        }
        saveActive |= PlotsList::instance().plotsList.at(idx).enabled;
    }
    if (saveActive) {
        saveBtn->setEnabled(true);
    }
    else {
        saveBtn->setEnabled(false);
    }
    show();
}

void FilesSaverDialog::on_save()
{
    FileSaver fs;
    QString dir = QFileDialog::getExistingDirectory(this, tr("Сохранить всё"),
                                                    "",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);


    if (!dir.isNull()) {
        QString filenames;
        QDateTime date = QDateTime::currentDateTime();
        for (int device = 0; device < devices.size(); ++device) {
            if (PlotsList::instance().plotsList.at(device).enabled && devices.at(device)->isChecked()) {
                int size = SignalData::instance(device).sizeSaved();
                if (size <= 0) break;
                std::vector<float> v(size);
                for (int idx = 0; idx < size; idx++) {
                    v[idx] = SignalData::instance(device).savedYValue(idx);
                }

                fs.saveFile(dir, PlotsList::instance().plotsList.at(device).channelName + "__" + date.toString("hhmmss"), v);
                filenames.append(PlotsList::instance().plotsList.at(device).channelName + "__" + date.toString("hhmmss") + "\n");
            }
        }
        if (filenames.size() > 0) {
            if (filesInfoCheckBox->isChecked()) {
                fs.generateConfig(dir, m_ltr11Freq, m_ltr22Freq);
            }
            QMessageBox::information(this, tr("Сохранение"),
                                       "Сохранены следующие данные: \n" + filenames,
                                       QMessageBox::Ok);
        }
        else QMessageBox::information(this, tr("Сохранение"),
                                      tr("Выберите данные для сохранения."),
                                      QMessageBox::Ok);
    }

}
