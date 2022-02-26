#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "./ModbusMaster/modbusmaster.h"
#include "Saving/filesaver.h"
#include "./Plots/plotter.h"
#include "./Settings/modbussettingsdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void on_openPlotsWindowAction();
    void on_saveAllAction();
    void on_startReceiveButtonClicked();
    void on_stopReceiveButtonClicked();
    void on_connectModbus();
    void on_writeModbus();
    void on_modbusStateChanged(int state);
    void toogleDevice(int row, int col);
    void on_deviceStatusChanged();
private:
    Ui::MainWindow *ui;

    ModbusMaster *m_modbus = nullptr;
    ModbusSettingsDialog *m_modbusSettings = nullptr;

    FileSaver *m_fs = nullptr;
    Plotter *m_plotter = nullptr;

    bool isStarted = false;

private:
    void initActions();
    void initDeviceTable();


};
#endif // MAINWINDOW_H
