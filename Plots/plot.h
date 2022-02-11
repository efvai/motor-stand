#ifndef PLOT_H
#define PLOT_H

#include <QwtPlot>
#include <QwtInterval>
#include <QElapsedTimer>

class QwtPlotCurve;
class QwtPlotMarker;
class QwtPlotDirectPainter;

class Plot : public QwtPlot
{
    Q_OBJECT
public:
    Plot(QWidget* = nullptr, int idx = 0);
    virtual ~Plot();
    void start();
    virtual void replot() QWT_OVERRIDE;

    virtual bool eventFilter( QObject*, QEvent* ) QWT_OVERRIDE;

public slots:
    void setIntervalLength( double );

protected:
    virtual void showEvent( QShowEvent* ) QWT_OVERRIDE;
    virtual void resizeEvent( QResizeEvent* ) QWT_OVERRIDE;
    virtual void timerEvent( QTimerEvent* ) QWT_OVERRIDE;

private:
    void updateCurve();
    void incrementInterval();

    QwtPlotMarker* m_origin;
    QwtPlotCurve* m_curve;
    int m_paintedPoints;

    QwtPlotDirectPainter* m_directPainter;

    QwtInterval m_interval;
    int m_timerId;

    QElapsedTimer m_elapsedTimer;
};

#endif // PLOT_H
