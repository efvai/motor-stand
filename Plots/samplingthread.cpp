#include "samplingthread.h"
#include "signaldata.h"

#include <QwtMath>
#include <qmath.h>
#include <QDebug>

SamplingThread::SamplingThread( QObject* parent )
    : QwtSamplingThread( parent )
    , m_frequency( 1.0 )
    , m_amplitude( 150.0 )
{
}

void SamplingThread::setFrequency( double frequency )
{
    m_frequency = frequency;
}

double SamplingThread::frequency() const
{
    return m_frequency;
}

void SamplingThread::setAmplitude( double amplitude )
{
    m_amplitude = amplitude;
}

double SamplingThread::amplitude() const
{
    return m_amplitude;
}

void SamplingThread::sample( double elapsed )
{
    if ( m_frequency > 0.0 )
    {
        const QPointF s( elapsed, value( elapsed ) );
        //const QPointF s1( elapsed, value(elapsed + 1));
        //SignalData::instance(0).append( s );
        SignalData::instance(1).append(s);
    }
}

double SamplingThread::value( double timeStamp ) const
{
    const double period = 1.0 / m_frequency;

    const double x = std::fmod( timeStamp, period );
    const double v = m_amplitude * qFastSin( x / period * 2 * M_PI );

    return v;
}
