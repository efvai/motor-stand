#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QAction>
#include <QDebug>
#include <QScreen>
#include <QFile>
#include <QFileDialog>
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
    m_modbus = new ModbusController();
    m_modbusSettings = new ModbusSettingsDialog(this);
    m_plotter = new Plotter();
    m_testThread = new SamplingThread();
    m_testThread->setInterval(10);
    m_fs = new FileSaver;
   // m_motorDisplay = ui->motorWidget;
    initActions();
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
        //->saveFile(dir, "test", plotsWindow->getMcuData());
    }

}

void MainWindow::on_startReceiveButtonClicked()
{
    // TODO : Check connections
    // Clear all values
    m_plotter->start();
    m_modbus->start();
    m_testThread->start();

}

void MainWindow::on_stopReceiveButtonClicked()
{
    //m_modbus->stop();
    m_testThread->stop();
}

void MainWindow::on_connectModbus()
{
    m_modbus->modbus->changeSettings(m_modbusSettings->settings());
    m_modbus->modbus->on_connect();
    m_modbus->init();
}

void MainWindow::on_writeModbus()
{
    m_modbus->modbus->setWriteValue(ui->targetSpinner->value());
    m_modbus->modbus->onWriteButtonClicked();
}

void MainWindow::on_modbusStateChanged(int state)
{
    if (state == 0)
        ui->connectButton->setText(tr("Подключиться"));
    else if (state == 1)
        ui->connectButton->setText(tr("Отключиться"));
}

void MainWindow::initActions()
{
    connect(ui->openPlotAction, &QAction::triggered, this, &MainWindow::on_openPlotsWindowAction);

    connect(ui->startTestBtn, &QPushButton::clicked, this, &MainWindow::on_startReceiveButtonClicked);
    connect(ui->stopTestBtn, &QPushButton::clicked, this, &MainWindow::on_stopReceiveButtonClicked);

    connect(ui->saveAllAction, &QAction::triggered, this, &MainWindow::on_saveAllAction);

    connect(ui->connectButton, &QPushButton::clicked,
            this, &MainWindow::on_connectModbus);
    connect(ui->settingsButton, &QPushButton::clicked,
            m_modbusSettings, &QDialog::show);
    connect(ui->writeButton, &QPushButton::clicked,
            this, &MainWindow::on_writeModbus);
    connect(ui->zeroButton, &QPushButton::clicked,
            m_modbus->modbus, &ModbusMaster::onZeroButtonClicked);
    connect(ui->enableBox, &QCheckBox::stateChanged,
            m_modbus->modbus, &ModbusMaster::onEnableBoxStateChanged);
    connect(m_modbus->modbus, &ModbusMaster::stateChanged, this, &MainWindow::on_modbusStateChanged);
}







