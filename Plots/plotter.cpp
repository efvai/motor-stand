#include "plotter.h"
#include <QLayout>
Plotter::Plotter(QWidget *parent) : QWidget(parent)
{
    m_ltr11Plot = new Plot(this, 0);
    m_testPlot = new Plot(this, 1);
    m_velPlot = new Plot(this, 2);
    QVBoxLayout* l = new QVBoxLayout();
    l->addWidget(m_ltr11Plot);
    l->addWidget(m_testPlot);
    l->addWidget(m_velPlot);
    setLayout(l);

}

void Plotter::start() {
    m_testPlot->start();
    m_ltr11Plot->start();
    m_velPlot->start();
}
