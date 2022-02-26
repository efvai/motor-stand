#ifndef PLOTTER_H
#define PLOTTER_H

#include <QWidget>
#include "plot.h"
#include "ltr11.h"
#include "ltr22.h"

class SamplingThread;
class GlobalPlotSettingsDialog;

class Plotter : public QWidget
{
    Q_OBJECT
public:
    Plotter(QWidget *parent = nullptr);
    ~Plotter();
    void start();
    void stop();
    Plot *m_testPlot = nullptr;
    Plot *m_ltr11Plot = nullptr;
    Plot *m_ltr22Plot = nullptr;
private:
    Ltr11 *m_ltr11 = nullptr;
    Ltr22 *m_ltr22 = nullptr;
    SamplingThread *m_test = nullptr;
    QList<Plot*> m_plots;

private:
    GlobalPlotSettingsDialog *m_settings;
    const int plotsCount = 6;
private slots:
    void on_statusChanged();
    void on_SettingsChanged();
public slots:
    void showSettings();
signals:
    void statusChanged();

};

#endif // PLOTTER_H
