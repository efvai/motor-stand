#include "plotswindow.h"
#include "ui_plotswindow.h"


#include <QDebug>
#include <QTimer>
#include <QtConcurrent/QtConcurrent>

ChartParams::ChartParams() :
    m_series(0),
    m_axisX(new RangeLimitedValueAxis()),
    m_axisY(new RangeLimitedValueAxis())
{

}

PlotsWindow::PlotsWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlotsWindow),
    thread(new RenderThread(this)),
    ltr11Thread(new Ltr11(this))
{
    testPlot = new ChartParams;
    ltr11Plot = new ChartParams;
    mcuPlot = new ChartParams;

    ui->setupUi(this);
    initTestPlot();
    initLtr11Plot();
    initMCUPlot();
    timer = new QTimer;
    connect(timer, &QTimer::timeout, this, &PlotsWindow::testRender);

}

PlotsWindow::~PlotsWindow()
{
    delete mcuPlot;
    delete testPlot;
    delete ltr11Plot;
    delete ui;
}

void PlotsWindow::initTestPlot() {
    testPlot->plot = ui->plotMotorCurrent->chart();
    testPlot->m_series = new QLineSeries();
    QPen green(Qt::black);
    green.setWidth(1);
    testPlot->m_series->setPen(green);
    testPlot->m_series->append(0, 0);
    testPlot->plot->addSeries(testPlot->m_series);
    testPlot->m_axisX->setLowerLimit(0);
    testPlot->m_axisX->setUpperLimit(5000);
    testPlot->m_axisY->setLowerLimit(-10);
    testPlot->m_axisY->setUpperLimit(10);
    //m_axisX->setTitleText("X");
    //m_axisY->setTitleText("Y");
    testPlot->plot->legend()->setVisible(false);

    testPlot->plot->addAxis(testPlot->m_axisX,Qt::AlignBottom);
    testPlot->plot->addAxis(testPlot->m_axisY,Qt::AlignLeft);
    testPlot->m_series->attachAxis(testPlot->m_axisX);
    testPlot->m_series->attachAxis(testPlot->m_axisY);
    testPlot->m_axisX->setTickCount(5);
    testPlot->m_axisX->setRange(0, 10);
    testPlot->m_axisY->setRange(-5, 10);
    connect(thread, &RenderThread::sendBlock,
            this, &PlotsWindow::addTestBlock);
}

void PlotsWindow::initLtr11Plot()
{
    ltr11Plot->plot = ui->plotLtr11->chart();
    ltr11Plot->m_series = new QLineSeries();
    QPen green(Qt::black);
    green.setWidth(1);
    ltr11Plot->m_series->setPen(green);
    ltr11Plot->m_series->append(0, 0);
    ltr11Plot->plot->addSeries(ltr11Plot->m_series);
    ltr11Plot->m_axisX->setLowerLimit(0);
    ltr11Plot->m_axisX->setUpperLimit(5000);
    ltr11Plot->m_axisY->setLowerLimit(-1);
    ltr11Plot->m_axisY->setUpperLimit(1);
    //m_axisX->setTitleText("X");
    //m_axisY->setTitleText("Y");
    ltr11Plot->plot->legend()->setVisible(false);

    ltr11Plot->plot->addAxis(ltr11Plot->m_axisX,Qt::AlignBottom);
    ltr11Plot->plot->addAxis(ltr11Plot->m_axisY,Qt::AlignLeft);
    ltr11Plot->m_series->attachAxis(ltr11Plot->m_axisX);
    ltr11Plot->m_series->attachAxis(ltr11Plot->m_axisY);
    ltr11Plot->m_axisX->setTickCount(5);
    ltr11Plot->m_axisX->setRange(0, 10);
    ltr11Plot->m_axisY->setRange(-1, 1);
    ltr11Plot->m_series->setUseOpenGL(true);
    connect(ltr11Thread, &Ltr11::sendBlock,
            this, &PlotsWindow::addLtr11Block);
}

void PlotsWindow::initMCUPlot() {
    mcuPlot->plot = ui->plotMotorVel->chart();
    mcuPlot->m_series = new QLineSeries();
    QPen green(Qt::black);
    green.setWidth(1);
    mcuPlot->m_series->setPen(green);
    mcuPlot->m_series->append(0, 0);
    mcuPlot->plot->addSeries(mcuPlot->m_series);
    mcuPlot->m_axisX->setLowerLimit(0);
    mcuPlot->m_axisX->setUpperLimit(5000);
    mcuPlot->m_axisY->setLowerLimit(-10);
    mcuPlot->m_axisY->setUpperLimit(10);
    //m_axisX->setTitleText("X");
    //m_axisY->setTitleText("Y");
    mcuPlot->plot->legend()->setVisible(false);

    mcuPlot->plot->addAxis(mcuPlot->m_axisX,Qt::AlignBottom);
    mcuPlot->plot->addAxis(mcuPlot->m_axisY,Qt::AlignLeft);
    mcuPlot->m_series->attachAxis(mcuPlot->m_axisX);
    mcuPlot->m_series->attachAxis(mcuPlot->m_axisY);
    mcuPlot->m_axisX->setTickCount(5);
    mcuPlot->m_axisX->setRange(0, 10);
    mcuPlot->m_axisY->setRange(-5, 10);
}

void PlotsWindow::testRender()
{
    //plotRender(m_testFreq, 1, testPlot, data);
    plotRender(m_ltr11Freq, 1, ltr11Plot, ltr11Data);
    //plotRender(m_mcuFreq, 1, mcuPlot, mcuData);
    //mcuPlot->plot->scroll(1, 0);
//    static int timeMarker = 0;
//    if (time == 0) timeMarker = 0;
//    int renderFreq = 20;

//    while (timeMarker < data.size()) {
//        testPlot->m_series->append(time, data.at(timeMarker));
//        time += (1.0 / renderFreq);
//        timeMarker+=renderFreq;
//    }
    //    testPlot->plot->scroll((1.0 / renderFreq), 0);
}

void PlotsWindow::plotRender(const int plotFreq, const int plotIter, ChartParams *plot, const std::vector<float> &data)
{
    std::size_t RenderSize = plotFreq / m_renderFreq;
    std::size_t Iter = plotIter;
    std::size_t IterSize = plot->m_series->count();
    float time = 0.0f;
    std::size_t idx = IterSize;
    if ((idx + RenderSize) >= data.size()) {
        qDebug() << "testRender() : no data to render";
        return;
    }
    while (idx < IterSize + RenderSize) {
        time = (float) idx/plotFreq;
        plot->m_series->append(time, data.at(idx));
        idx += Iter;
    }

}

std::vector<float> PlotsWindow::getMcuData() const
{
    return mcuData;
}

void PlotsWindow::addTestBlock(const std::vector<double> &block) {
    //qreal x = plot->plotArea().width() / m_axisX->tickCount();
    //testPlot->m_series->append(block);
    data.insert(data.end(), block.begin(), block.end());
    //qDebug() << data.size();
//    for (auto sample : block) {

//        time += 1e-2;

//        testPlot->m_series->append(time, sample);
//    }
    //testPlot->plot->scroll(10, 0);
}

void PlotsWindow::addLtr11Block(const std::vector<double> &block) {
    //qDebug() << "addLtr11BLock size of : " << block.size();
    ltr11Data.insert(ltr11Data.end(), block.begin(), block.end());
//    static float time = 0.0;
//    int i = 0;
//    for (auto sample : block) {
//        time += 1e-4;
//        ltr11Plot->m_series->append(time, sample);
//        i++;
//        if (i > 1000) break;
//    }
}

void PlotsWindow::addMCUBlock(const float block)
{
   mcuData.push_back(block);
}

void PlotsWindow::startTestPlot() {
    clearPlots();
    //thread->processGraph();
    startLtr11Plot();
    timer->start(100);
}

void PlotsWindow::stopTestPlot() {
    thread->stopProcess();
    stopLtr11Plot();
    timer->stop();
    qDebug() << "mcu Data elements: " << ltr11Data.size();
    qDebug() << "mcu Plot elements: " << ltr11Plot->m_series->count();
}

void PlotsWindow::startLtr11Plot()
{
    ltr11Thread->processReceive();
}

void PlotsWindow::stopLtr11Plot()
{
    ltr11Thread->stopProcess();
}

void PlotsWindow::clearPlots()
{
    ticks = 0;
    data.clear();
    mcuData.clear();
    ltr11Data.clear();
    testPlot->m_series->clear();
    ltr11Plot->m_series->clear();
    mcuPlot->m_series->clear();
}


