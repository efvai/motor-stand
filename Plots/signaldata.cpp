#include "signaldata.h"

#include <QVector>
#include <QMutex>
#include <QReadWriteLock>
#include <QDebug>

class SignalData::PrivateData
{
  public:
    PrivateData()
        : boundingRect( 1.0, 1.0, -2.0, -2.0 ) // invalid
    {
        values.reserve( 1000 );
        saveValues.reserve( 1000 );
    }

    inline void append( const QPointF& sample )
    {
        values.append( sample );

        // adjust the bounding rectangle

        if ( boundingRect.width() < 0 || boundingRect.height() < 0 )
        {
            boundingRect.setRect( sample.x(), sample.y(), 0.0, 0.0 );
        }
        else
        {
            boundingRect.setRight( sample.x() );

            if ( sample.y() > boundingRect.bottom() )
                boundingRect.setBottom( sample.y() );

            if ( sample.y() < boundingRect.top() )
                boundingRect.setTop( sample.y() );
        }
    }

    inline void appendSave()
    {
        saveValues.append(values);
    }

    QReadWriteLock lock;

    QVector< QPointF > values;
    QVector< QPointF > saveValues;
    QRectF boundingRect;

    QMutex mutex; // protecting pendingValues
    QVector< QPointF > pendingValues;


};

SignalData::SignalData()
{
    m_data = new PrivateData();
}

SignalData::~SignalData()
{
    delete m_data;
}

int SignalData::size() const
{
    return m_data->values.size();
}

int SignalData::sizeSaved() const
{
    return m_data->saveValues.size();
}

QPointF SignalData::value( int index ) const
{
    return m_data->values[index];
}

float SignalData::savedYValue( int index) const
{
    return m_data->saveValues[index].y();
}

QRectF SignalData::boundingRect() const
{
    return m_data->boundingRect;
}

void SignalData::lock()
{
    m_data->lock.lockForRead();
}

void SignalData::unlock()
{
    m_data->lock.unlock();
}

void SignalData::append( const QPointF& sample )
{
    m_data->mutex.lock();
    m_data->pendingValues += sample;

    const bool isLocked = m_data->lock.tryLockForWrite();
    if ( isLocked )
    {
        const int numValues = m_data->pendingValues.size();
        const QPointF* pendingValues = m_data->pendingValues.data();

        for ( int i = 0; i < numValues; i++ )
            m_data->append( pendingValues[i] );

        m_data->pendingValues.clear();
        m_data->lock.unlock();
    }

    m_data->mutex.unlock();
}

void SignalData::clearStaleValues( double limit )
{
    m_data->lock.lockForWrite();

    m_data->boundingRect = QRectF( 1.0, 1.0, -2.0, -2.0 ); // invalid

    const QVector< QPointF > values = m_data->values;
    m_data->appendSave();
    m_data->values.clear();
    m_data->values.reserve( values.size() );

    int index;
    for ( index = values.size() - 1; index >= 0; index-- )
    {
        if ( values[index].x() < limit )
            break;
    }

    if ( index > 0 )
        m_data->append( values[index++] );

    while ( index < values.size() - 1 )
        m_data->append( values[index++] );

    m_data->lock.unlock();
}

void SignalData::clearValues()
{
    m_data->lock.lockForWrite();

    m_data->boundingRect = QRectF( 1.0, 1.0, -2.0, -2.0 ); // invalid

    std::size_t sizeOfValues = m_data->values.size();
    std::size_t sizeOfSave = m_data->saveValues.size();
    m_data->values.clear();
    m_data->values.reserve( sizeOfValues );
    m_data->saveValues.clear();
    m_data->saveValues.reserve(sizeOfSave);
    m_data->lock.unlock();
}

void SignalData::save()
{
    m_data->appendSave();
}

void SignalData::assignSaved()
{
    m_data->values = m_data->saveValues;
}

SignalData& SignalData::instance(int idx)
{
    static SignalData valueVector[7];
    return valueVector[idx];
}



