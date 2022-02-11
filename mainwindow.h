#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "./Plots/plotswindow.h"
#include "./modbusMasterInc/modbusMaster/modbusmaster.h"
#include "Saving/filesaver.h"
#include "./Plots/plotter.h"
#include "./Plots/samplingthread.h"

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
private:
    Ui::MainWindow *ui;
    ModbusMaster *m_motorDisplay = nullptr;
    FileSaver *m_fs = nullptr;
    Plotter *m_plotter = nullptr;
    SamplingThread *m_testThread = nullptr;


};
#endif // MAINWINDOW_H
