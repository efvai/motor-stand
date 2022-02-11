#ifndef SAMPLINGTHREAD_H
#define SAMPLINGTHREAD_H

#include "QwtSamplingThread"

class SamplingThread : public QwtSamplingThread
{
    Q_OBJECT
public:
  SamplingThread( QObject* parent = NULL );

  double frequency() const;
  double amplitude() const;

public slots:
  void setAmplitude( double );
  void setFrequency( double );

protected:
  virtual void sample( double elapsed ) QWT_OVERRIDE;

private:
  virtual double value( double timeStamp ) const;

  double m_frequency;
  double m_amplitude;
};

#endif // SAMPLINGTHREAD_H
