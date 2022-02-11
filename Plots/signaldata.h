#ifndef SIGNALDATA_H
#define SIGNALDATA_H


#include <QRect>

class SignalData
{
  public:
    static SignalData& instance(int idx);

    void append( const QPointF& pos );
    void clearStaleValues( double min );

    int size() const;
    QPointF value( int index ) const;

    QRectF boundingRect() const;

    void lock();
    void unlock();

  private:
    SignalData();
    ~SignalData();

    Q_DISABLE_COPY( SignalData )

    class PrivateData;
    PrivateData* m_data;
};

#endif // SIGNALDATA_H
