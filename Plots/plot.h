#ifndef PLOT_H
#define PLOT_H

#include <QwtPlot>
#include <QwtInterval>
#include <QElapsedTimer>
#include <QwtPlotCurve>

class QwtPlotCurve;
class QwtPlotMarker;
class QwtPlotDirectPainter;
class QwtPlotLegendItem;


QT_BEGIN_NAMESPACE
class QAction;
class QActionGroup;
class QMenu;
QT_END_NAMESPACE

class PlotSettingsDialog;
class NamedPlotCurve : public QwtPlotCurve {

public:
    const QString &getName() const;
    void setName(const QString &newName);
    int getIndex() const;
    void setIndex(int newIndex);

private:
    QString name;
    int index;
};

class Plot : public QwtPlot
{
    Q_OBJECT
public:
    Plot(QWidget* = nullptr, int idx = 0);
    virtual ~Plot();
    void start();
    void stop();
    void clear();
    virtual void replot() QWT_OVERRIDE;

    virtual bool eventFilter( QObject*, QEvent* ) QWT_OVERRIDE;

    bool getIsStarted() const;

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
    NamedPlotCurve* m_curve;
    int m_paintedPoints;

    QwtPlotDirectPainter* m_directPainter;

    QwtInterval m_interval;
    int m_timerId;

    QElapsedTimer m_elapsedTimer;
    bool elapsedTimerStarted = false;

    QMenu *mainMenu;
    QMenu *dataSubMenu;
    QAction *settings;
    PlotSettingsDialog *m_settings = nullptr;
    QActionGroup *newData = nullptr;

    QwtPlotLegendItem *m_legendItem = nullptr;
    bool isInit = true;
    bool isStarted = false;
private slots:
    void updateSettings();
    void setNewData(QAction *triggered);
};

#endif // PLOT_H
