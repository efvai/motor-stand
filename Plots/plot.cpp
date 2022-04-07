#include "plot.h"
#include "signaldata.h"
#include "./Settings/plotsettingsdialog.h"

#include <QwtPlotGrid>
#include <QwtPlotLayout>
#include <QwtPlotCanvas>
#include <QwtPlotMarker>
//#include <QwtPlotCurve>
#include <QwtScaleDiv>
#include <QwtScaleMap>
#include <QwtPlotDirectPainter>
#include <QwtPainter>
#include <QwtPlotLegendItem>

#include <QEvent>
#include <QAction>
#include <QMenu>
#include <QMouseEvent>

#include <QDebug>

namespace
{
class LegendItem : public QwtPlotLegendItem
{
public:
    LegendItem()
    {
        setRenderHint( QwtPlotItem::RenderAntialiased );

        const QColor c1( Qt::black );

        setTextPen( c1 );
        setBorderPen( c1 );

        QColor c2( Qt::white );
        c2.setAlpha( 200 );

        setBackgroundBrush( c2 );
    }
};
class Canvas : public QwtPlotCanvas
{
public:
    Canvas( QwtPlot* plot = NULL )
        : QwtPlotCanvas( plot )
    {
        /*
                The backing store is important, when working with widget
                overlays ( f.e rubberbands for zooming ).
                Here we don't have them and the internal
                backing store of QWidget is good enough.
             */

        setPaintAttribute( QwtPlotCanvas::BackingStore, false );
        setBorderRadius( 10 );

        if ( QwtPainter::isX11GraphicsSystem() )
        {
#if QT_VERSION < 0x050000
            /*
                    Qt::WA_PaintOutsidePaintEvent works on X11 and has a
                    nice effect on the performance.
                 */

            setAttribute( Qt::WA_PaintOutsidePaintEvent, true );
#endif

            /*
                    Disabling the backing store of Qt improves the performance
                    for the direct painter even more, but the canvas becomes
                    a native window of the window system, receiving paint events
                    for resize and expose operations. Those might be expensive
                    when there are many points and the backing store of
                    the canvas is disabled. So in this application
                    we better don't disable both backing stores.
                 */

            if ( testPaintAttribute( QwtPlotCanvas::BackingStore ) )
            {
                setAttribute( Qt::WA_PaintOnScreen, true );
                setAttribute( Qt::WA_NoSystemBackground, true );
            }
        }

        setupPalette();
    }

private:
    void setupPalette()
    {
        QPalette pal = palette();

        pal.setBrush( QPalette::Window, QBrush( Qt::white ) );

        // QPalette::WindowText is used for the curve color
        pal.setColor( QPalette::WindowText, Qt::black );

        setPalette( pal );
    }
};

class CurveData : public QwtSeriesData< QPointF >
{
public:
    const SignalData& values() const
    {
        return SignalData::instance(idx);
    }

    SignalData& values()
    {
        return SignalData::instance(idx);
    }

    virtual QPointF sample( size_t index ) const QWT_OVERRIDE
    {
        return SignalData::instance(idx).value( index );
    }

    virtual size_t size() const QWT_OVERRIDE
    {
        return SignalData::instance(idx).size();
    }

    virtual QRectF boundingRect() const QWT_OVERRIDE
    {
        return SignalData::instance(idx).boundingRect();
    }
    void setIndex(int idx) {
        this->idx = idx;
    }
    int getIndex() {
        return idx;
    }
private:
    int idx;
};

}

Plot::Plot(QWidget* parent, int idx) :
    QwtPlot(parent)
  ,m_paintedPoints( 0 )
  ,m_interval( 0.0, 10.0 )
  ,m_timerId( -1 )
{
    settings = new QAction(tr("&Параметры графика"), this);
    m_settings = new PlotSettingsDialog(this);
    connect(settings, &QAction::triggered, m_settings, &QDialog::show);
    connect(m_settings, &PlotSettingsDialog::settingsChanged, this, &Plot::updateSettings);
    newData = new QActionGroup(this);
    newData->setExclusive(false);
    mainMenu = new QMenu(this);
    dataSubMenu = new QMenu(this);
    dataSubMenu->setTitle("Выбрать данные:");
    for (int jdx = 0; jdx < PlotsList::instance().plotsList.size(); jdx++) {
        newData->addAction(dataSubMenu->addAction(
                               PlotsList::instance().plotsList.at(jdx).channelName))->setData(
                               PlotsList::instance().plotsList.at(jdx).index);
    }
    connect(newData, &QActionGroup::triggered, this, &Plot::setNewData);
    mainMenu->addMenu(dataSubMenu);
    mainMenu->addAction(settings);



    m_directPainter = new QwtPlotDirectPainter();

    setAutoReplot( false );
    setCanvas( new Canvas() );

    plotLayout()->setAlignCanvasToScales( true );

    //setAxisTitle( QwtAxis::XBottom, "Время [с]" );
    setAxisScale( QwtAxis::XBottom, m_interval.minValue(), m_interval.maxValue() );

    QwtPlotGrid* grid = new QwtPlotGrid();
    grid->setPen( Qt::gray, 0.0, Qt::DotLine );
    grid->enableX( true );
    grid->enableXMin( true );
    grid->enableY( true );
    grid->enableYMin( false );
    grid->attach( this );

    m_origin = new QwtPlotMarker();
    m_origin->setLineStyle( QwtPlotMarker::Cross );
    m_origin->setValue( m_interval.minValue() + m_interval.width() / 2.0, 0.0 );
    m_origin->setLinePen( Qt::gray, 0.0, Qt::DashLine );
    m_origin->attach( this );

    m_curve = new NamedPlotCurve();
    m_curve->setStyle( QwtPlotCurve::Lines );
    m_curve->setPen( Qt::red , 1, Qt::SolidLine);
    m_curve->setRenderHint( QwtPlotItem::RenderAntialiased, true );
    m_curve->setPaintAttribute( QwtPlotCurve::ClipPolygons, false );
    m_curve->setIndex(idx);
    m_curve->setTitle(m_curve->getName());
    CurveData *data = new CurveData();
    data->setIndex(idx);
    m_curve->setData( data );

    m_curve->attach( this );

    updateSettings();
    isInit = false;
}


Plot::~Plot()
{
    delete m_directPainter;
}



void Plot::start()
{
    if (PlotsList::instance().plotsList.at(m_curve->getIndex()).enabled) {
        if (!elapsedTimerStarted)
            m_elapsedTimer.start();
        else {
            m_elapsedTimer.restart();
            clear();
        }
        m_timerId = startTimer( 10 );
        isStarted = true;
    }
    else {
        clear();
    }
}

void Plot::stop() {
    if (PlotsList::instance().plotsList.at(m_curve->getIndex()).enabled) {
        killTimer(m_timerId);
        elapsedTimerStarted = true;
        CurveData* curveData = static_cast< CurveData* >( m_curve->data() );
        curveData->values().save();
        curveData->values().assignSaved();
        if (curveData->values().size() > 0) {
            m_interval.setInterval(0.0, curveData->values().value(curveData->values().size() - 1).x());
            setAxisScale( QwtAxis::XBottom, m_interval.minValue(), m_interval.maxValue() );
        }
        isStarted = false;
        replot();
    }
}

void Plot::clear()
{
    CurveData* curveData = static_cast< CurveData* >( m_curve->data() );
    curveData->values().clearValues();
    m_interval.setInterval(m_interval.minValue(), m_settings->settings().xMax + m_interval.minValue());
    setAxisScale( QwtAxis::XBottom, m_interval.minValue(), m_interval.maxValue() );
    replot();
}

void Plot::replot()
{
    CurveData* curveData = static_cast< CurveData* >( m_curve->data() );
    curveData->values().lock();

    QwtPlot::replot();
    m_paintedPoints = curveData->size();

    curveData->values().unlock();
}

void Plot::setIntervalLength( double interval )
{
    if ( interval > 0.0 && interval != m_interval.width() )
    {
        m_interval.setMaxValue( m_interval.minValue() + interval );
        setAxisScale( QwtAxis::XBottom,
            m_interval.minValue(), m_interval.maxValue() );

        replot();
    }
}

void Plot::updateCurve()
{
    CurveData* curveData = static_cast< CurveData* >( m_curve->data() );
    curveData->values().lock();

    const int numPoints = curveData->size();
    if ( numPoints > m_paintedPoints )
    {
        const bool doClip = !canvas()->testAttribute( Qt::WA_PaintOnScreen );
        if ( doClip )
        {
            /*
                Depending on the platform setting a clip might be an important
                performance issue. F.e. for Qt Embedded this reduces the
                part of the backing store that has to be copied out - maybe
                to an unaccelerated frame buffer device.
             */

            const QwtScaleMap xMap = canvasMap( m_curve->xAxis() );
            const QwtScaleMap yMap = canvasMap( m_curve->yAxis() );

            QRectF br = qwtBoundingRect( *curveData,
                m_paintedPoints - 1, numPoints - 1 );

            const QRect clipRect = QwtScaleMap::transform( xMap, yMap, br ).toRect();
            m_directPainter->setClipRegion( clipRect );
        }

        m_directPainter->drawSeries( m_curve,
            m_paintedPoints - 1, numPoints - 1 );
        m_paintedPoints = numPoints;
    }

    curveData->values().unlock();
}

void Plot::incrementInterval()
{
    m_interval = QwtInterval( m_interval.maxValue(),
        m_interval.maxValue() + m_interval.width() );

    CurveData* curveData = static_cast< CurveData* >( m_curve->data() );
    curveData->values().clearStaleValues( m_interval.minValue() );

    // To avoid, that the grid is jumping, we disable
    // the autocalculation of the ticks and shift them
    // manually instead.

    QwtScaleDiv scaleDiv = axisScaleDiv( QwtAxis::XBottom );
    scaleDiv.setInterval( m_interval );

    for ( int i = 0; i < QwtScaleDiv::NTickTypes; i++ )
    {
        QList< double > ticks = scaleDiv.ticks( i );
        for ( int j = 0; j < ticks.size(); j++ )
            ticks[j] += m_interval.width();
        scaleDiv.setTicks( i, ticks );
    }
    setAxisScaleDiv( QwtAxis::XBottom, scaleDiv );

    m_origin->setValue( m_interval.minValue() + m_interval.width() / 2.0, 0.0 );

    m_paintedPoints = 0;
    replot();
}

bool Plot::getIsStarted() const
{
    return isStarted;
}

void Plot::updateSettings()
{
    setAxisScale( QwtAxis::YLeft, m_settings->settings().yMin, m_settings->settings().yMax);
    m_interval.setInterval(m_interval.minValue(), m_settings->settings().xMax + m_interval.minValue());
    setAxisScale( QwtAxis::XBottom, m_interval.minValue(), m_interval.maxValue() );
    if (m_settings->settings().legendEnabled) {
        if (m_legendItem == nullptr) {
            m_legendItem = new LegendItem();
            m_legendItem->attach(this);
        }
        m_legendItem->setAlignmentInCanvas( Qt::Alignment(m_settings->settings().legendAlignment | Qt::AlignTop));
        m_legendItem->setBorderRadius( 8 );
        m_legendItem->setMargin( 4 );
        m_legendItem->setSpacing( 2 );
        m_legendItem->setItemMargin( 0 );
    }
    else {
        delete m_legendItem;
        m_legendItem = nullptr;
    }
    QwtPlotItemList curveList = itemList(QwtPlotItem::Rtti_PlotCurve);
    for (int idx = 0; idx < curveList.count(); idx++) {
        NamedPlotCurve *curve = static_cast<NamedPlotCurve*>(curveList[idx]);
        curve->setLegendIconSize(QSize(8, 8));
        curve->setTitle(curve->getName());

    }
    if (!isInit)
        replot();
}

void Plot::setNewData(QAction *triggered)
{
    int key = triggered->data().toInt();
    if (key != m_curve->getIndex()) {
    CurveData *data = new CurveData();
    data->setIndex(key);
    m_curve->setIndex(key);
    m_curve->swapData( data );
    }
    updateSettings();
}

void Plot::timerEvent( QTimerEvent* event )
{
    if ( event->timerId() == m_timerId )
    {
        updateCurve();

        const double elapsed = m_elapsedTimer.elapsed() / 1e3;
        if ( elapsed > m_interval.maxValue() )
            incrementInterval();

        return;
    }

    QwtPlot::timerEvent( event );
}

void Plot::resizeEvent( QResizeEvent* event )
{
    m_directPainter->reset();
    QwtPlot::resizeEvent( event );
}

void Plot::showEvent( QShowEvent* )
{
    replot();
}

bool Plot::eventFilter( QObject* object, QEvent* event )
{
    if ( object == canvas() &&
        event->type() == QEvent::PaletteChange )
    {
        m_curve->setPen( canvas()->palette().color( QPalette::WindowText ) );
    }
    if (event->type() == QEvent::ContextMenu) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*> (event);
        if (isStarted) {
            dataSubMenu->setEnabled(false);
        } else {
            dataSubMenu->setEnabled(true);
        }
        mainMenu->exec(mouseEvent->globalPos());
    }

    return QwtPlot::eventFilter( object, event );
}

const QString &NamedPlotCurve::getName() const
{
    return name;
}

void NamedPlotCurve::setName(const QString &newName)
{
    name = newName;
}

int NamedPlotCurve::getIndex() const
{
    return index;
}

void NamedPlotCurve::setIndex(int newIndex)
{
    index = newIndex;
    name = PlotsList::instance().plotsList.at(index).channelName;
}
