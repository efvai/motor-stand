#ifndef PLOTSWINDOW_H
#define PLOTSWINDOW_H

#include <QWidget>
#include "zoomablechartwidget.h"
#include "rangelimitedvalueaxis.h"
#include "renderthread.h"
#include "ltr11.h"
#include "ltr22.h"

namespace Ui {
class PlotsWindow;
}
QT_CHARTS_BEGIN_NAMESPACE
class QSplineSeries;
class QValueAxis;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

class ChartParams
{
public:
    QChart *plot;
    QLineSeries *m_series;
    RangeLimitedValueAxis *m_axisX;
    RangeLimitedValueAxis *m_axisY;
    ChartParams();
};


class PlotsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PlotsWindow(QWidget *parent = nullptr);
    ~PlotsWindow();

private:
    Ui::PlotsWindow *ui;

    ChartParams *testPlot;
    ChartParams *ltr11Plot;
    ChartParams *mcuPlot;

    RenderThread *thread;
    Ltr11 *ltr11Thread;

public slots:
    void addTestBlock(const std::vector<double> &block);
    void addLtr11Block(const std::vector<double> &block);
    void addMCUBlock(const float block);
    void startTestPlot();
    void stopTestPlot();
    void startLtr11Plot();
    void stopLtr11Plot();
    void clearPlots();

private:
    void initTestPlot();
    void initLtr11Plot();
    void initLtr22Plot();
    void initMCUPlot();


    std::vector<float> data;
    std::vector<float> mcuData;
    std::vector<float> ltr11Data;
private:
    void testRender();
    void plotRender(const int plotFreq, const int plotIter, ChartParams *plot, const std::vector<float> &data);


    std::size_t ticks = 0;
    QTimer *timer;
    const int m_renderFreq = 10;
    const int m_testFreq = 60;
    const int m_mcuFreq = 100;
    const int m_ltr11Freq = 10000;


public:
    std::vector<float> getMcuData() const;

};

#endif // PLOTSWINDOW_H
