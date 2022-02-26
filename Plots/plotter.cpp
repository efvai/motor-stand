#include "plotter.h"
#include "samplingthread.h"
#include "signaldata.h"
#include <QDebug>
#include <QLayout>
#include <./Settings/globalplotsettingsdialog.h>
Plotter::Plotter(QWidget *parent) : QWidget(parent)
{
    //m_ltr11Plot = new Plot(this, 0);
    //m_testPlot = new Plot(this, 1);
    //m_ltr22Plot = new Plot(this, 2);
    m_settings = new GlobalPlotSettingsDialog();


    m_ltr11 = new Ltr11(this);
    m_ltr22 = new Ltr22(this);
    m_test = new SamplingThread();
    m_test->setInterval(10);

    QVBoxLayout* l = new QVBoxLayout();
    for (int idx = 0; idx < plotsCount; idx++) {
        m_plots.append(new Plot(this, idx));
        l->addWidget(m_plots[idx]);
    }
    on_SettingsChanged();
    //l->addWidget(m_ltr11Plot);
    //l->addWidget(m_testPlot);
    //l->addWidget(m_ltr22Plot);
    setLayout(l);
    connect(m_ltr11, &Ltr11::sendStatus, this, &Plotter::on_statusChanged);
    connect(m_ltr22, &Ltr22::sendStatus, this, &Plotter::on_statusChanged);
    connect(m_settings, &GlobalPlotSettingsDialog::settingsChanged, this, &Plotter::on_SettingsChanged);
}

Plotter::~Plotter() {
    m_ltr11->closeLtr();
}

void Plotter::start() {
//    m_testPlot->start();
//    //m_test->start();
//    m_ltr11Plot->start();
//    m_ltr22Plot->start();
    //m_velPlot->start();
    for (auto plot : m_plots) {
        if (!plot->isHidden())
            plot->start();
    }
    m_ltr11->transaction();
    m_ltr22->transaction();

}

void Plotter::stop() {
    m_ltr11->setPause(true);
    m_ltr22->setPause(true);
//    m_ltr11Plot->stop();
//    m_ltr22Plot->stop();
//    m_testPlot->stop();
}

void Plotter::on_statusChanged()
{
    emit statusChanged();
}

void Plotter::on_SettingsChanged()
{
    const int count = m_settings->settings().plotsCount;
    for (int idx = count; idx < plotsCount; idx++) {
        m_plots[idx]->hide();
    }
    for (int idx = 0; idx < count; idx++) {
        m_plots[idx]->show();
    }
}

void Plotter::showSettings()
{
    m_settings->show();
}
