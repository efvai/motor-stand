#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "./Plots/plotswindow.h"
#include "./ModbusMaster/modbusmaster.h"
#include "Saving/filesaver.h"
#include "./Plots/plotter.h"
#include "./Plots/samplingthread.h"
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
private:
    Ui::MainWindow *ui;

    ModbusController *m_modbus = nullptr;
    ModbusSettingsDialog *m_modbusSettings = nullptr;

    FileSaver *m_fs = nullptr;
    Plotter *m_plotter = nullptr;
    SamplingThread *m_testThread = nullptr;

private:
    void initActions();


};
#endif // MAINWINDOW_H
