#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QAction>
#include <QDebug>
#include <QScreen>
#include <QFile>
#include <QFileDialog>
#include <QStringList>

#include "./Plots/signaldata.h"
#include "./Plots/samplingthread.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    QRect windowGeometry = QRect(screenGeometry.x() + 50, screenGeometry.y()+screenGeometry.height()/3, screenGeometry.width()/2, screenGeometry.height()/2);
    this->setGeometry(windowGeometry);
    this->setMinimumSize(windowGeometry.width()/2, windowGeometry.height()/2);
    //this->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(this);
    m_modbus = new ModbusMaster();
    m_modbusSettings = new ModbusSettingsDialog(this);
    m_plotter = new Plotter();
    m_fs = new FileSaver;
   // m_motorDisplay = ui->motorWidget;
    initDeviceTable();
    initActions();
    on_deviceStatusChanged();
}

MainWindow::~MainWindow()
{
    delete m_modbus;
    delete m_fs;
    delete ui;
}

void MainWindow::on_openPlotsWindowAction() {
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    QRect windowGeometry = QRect(screenGeometry.x()+screenGeometry.width()/2, screenGeometry.y()+screenGeometry.height()/10, screenGeometry.width()/2, screenGeometry.height()/1.2);
    qDebug() << "window geom" << screenGeometry << "\n";
    qDebug() << "window geom" << windowGeometry << "\n";
//    plotsWindow->setGeometry(windowGeometry);
//    plotsWindow->setMinimumSize(screenGeometry.width()/2, screenGeometry.height()/1.2);
//    plotsWindow->show();
    m_plotter->setGeometry(windowGeometry);
    m_plotter->setMinimumSize(screenGeometry.width()/2, screenGeometry.height()/1.2);
    m_plotter->show();
}

void MainWindow::on_saveAllAction()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Сохранить всё"),
                                                    "",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    if (!dir.isNull()) {
        qDebug() << dir;
        int size = SignalData::instance(0).sizeSaved();
        if (size <= 0) return;
        std::vector<float> v(size);
        for (int idx = 0; idx < size; idx++) {
            v[idx] = SignalData::instance(0).savedYValue(idx);
        }
        m_fs->saveFile(dir, "LTR11", v);
    }

}

void MainWindow::on_startReceiveButtonClicked()
{
    // TODO : Check connections
    // Clear all values
    m_plotter->start();
    ui->stopTestBtn->setEnabled(true);
    ui->startTestBtn->setEnabled(false);
    isStarted = true;

}

void MainWindow::on_stopReceiveButtonClicked()
{
    //m_modbus->stop();
    m_plotter->stop();
    ui->stopTestBtn->setEnabled(false);
    ui->startTestBtn->setEnabled(true);
    isStarted = false;
}

void MainWindow::on_connectModbus()
{
    m_modbus->changeSettings(m_modbusSettings->settings());
    m_modbus->on_connect();
    //m_modbus->init();
}

void MainWindow::on_writeModbus()
{
    //m_modbus->setWriteValue(ui->targetSpinner->value());
    m_modbus->onWriteButtonClicked();
}

void MainWindow::on_modbusStateChanged(int state)
{
    //if (state == 0)
       // ui->connectButton->setText(tr("Подключиться"));
    //else if (state == 1)
       // ui->connectButton->setText(tr("Отключиться"));
}

void MainWindow::toogleDevice(int row, int col)
{
    if (!isStarted) {
        if (col == 2) {
            qDebug() << row;
            if (!PlotsList::instance().plotsList.at(row).enabled) {
                ui->deviceTable->setItem(row, 2, new QTableWidgetItem("Вкл."));
                ui->deviceTable->item(row, 2)->setBackground(Qt::green);
                PlotsList::instance().setEnabled(row, true);
            }
            else {
                ui->deviceTable->setItem(row, 2, new QTableWidgetItem("Откл."));
                ui->deviceTable->item(row, 2)->setBackground(Qt::red);
                PlotsList::instance().setEnabled(row, false);
            }
            ui->deviceTable->resizeRowsToContents();
        }
    }
}

void MainWindow::on_deviceStatusChanged()
{
    //qDebug() << "HEHEY";
    for (int i = 0; i < ui->deviceTable->rowCount(); i++) {
        ui->deviceTable->setItem(i, 3, new QTableWidgetItem(PlotsList::instance().plotsList.at(i).status));
        //qDebug() << PlotsList::instance().plotsList.at(i).status;

    }
    ui->deviceTable->resizeRowsToContents();
}

void MainWindow::initActions()
{
    connect(ui->openPlotAction, &QAction::triggered, this, &MainWindow::on_openPlotsWindowAction);
    connect(ui->openPlotBtn, &QPushButton::clicked, this, &MainWindow::on_openPlotsWindowAction);

    connect(ui->globalPlotSettingsAction, &QAction::triggered, m_plotter, &Plotter::showSettings);

    connect(ui->startTestBtn, &QPushButton::clicked, this, &MainWindow::on_startReceiveButtonClicked);
    connect(ui->stopTestBtn, &QPushButton::clicked, this, &MainWindow::on_stopReceiveButtonClicked);

    connect(ui->saveAllAction_2, &QAction::triggered, this, &MainWindow::on_saveAllAction);

//    connect(ui->connectButton, &QPushButton::clicked,
//            this, &MainWindow::on_connectModbus);
//    connect(ui->settingsButton, &QPushButton::clicked,
//            m_modbusSettings, &QDialog::show);
//    connect(ui->writeButton, &QPushButton::clicked,
//            this, &MainWindow::on_writeModbus);
//    connect(ui->zeroButton, &QPushButton::clicked,
//            m_modbus, &ModbusMaster::onZeroButtonClicked);
//    connect(ui->enableBox, &QCheckBox::stateChanged,
//            m_modbus, &ModbusMaster::onEnableBoxStateChanged);
    connect(m_modbus, &ModbusMaster::stateChanged, this, &MainWindow::on_modbusStateChanged);

    connect(m_plotter, &Plotter::statusChanged, this, &MainWindow::on_deviceStatusChanged);
    ui->stopTestBtn->setEnabled(false);
}

void MainWindow::initDeviceTable()
{
    QStringList headers;
    headers << "Устройство" << "Канал" << "Вкл/Откл" << "Состояние";
    int rowCount = PlotsList::instance().plotsList.size();
    ui->deviceTable->setRowCount(rowCount);
    ui->deviceTable->setColumnCount(headers.size());
    ui->deviceTable->setHorizontalHeaderLabels(headers);
    ui->deviceTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->deviceTable->setSelectionMode(QAbstractItemView::SingleSelection);
    for (int idx = 0; idx < rowCount; idx++) {
        ui->deviceTable->setItem(idx, 0, new QTableWidgetItem(PlotsList::instance().plotsList.at(idx).name));
        ui->deviceTable->setItem(idx, 1, new QTableWidgetItem(PlotsList::instance().plotsList.at(idx).channelName));
        if (PlotsList::instance().plotsList.at(idx).enabled) {
            ui->deviceTable->setItem(idx, 2, new QTableWidgetItem("Вкл."));
            ui->deviceTable->item(idx, 2)->setBackground(Qt::green);
        }
        else {
            ui->deviceTable->setItem(idx, 2, new QTableWidgetItem("Откл."));
            ui->deviceTable->item(idx, 2)->setBackground(Qt::red);
        }

    }
    ui->deviceTable->resizeColumnsToContents();
    ui->deviceTable->resizeRowsToContents();
    ui->deviceTable->horizontalHeader()->setStretchLastSection(true);
    ui->deviceTable->horizontalHeader()->setSectionsClickable(false);
    ui->deviceTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->deviceTable->verticalHeader()->setSectionsClickable(false);
    connect(ui->deviceTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::toogleDevice);

}







