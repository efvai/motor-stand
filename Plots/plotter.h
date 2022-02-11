#ifndef PLOTTER_H
#define PLOTTER_H

#include <QWidget>
#include "plot.h"

class Plotter : public QWidget
{
    Q_OBJECT
public:
    Plotter(QWidget *parent = nullptr);
    void start();
    Plot *m_testPlot = nullptr;
    Plot *m_ltr11Plot = nullptr;
    Plot *m_velPlot = nullptr;

};

#endif // PLOTTER_H
