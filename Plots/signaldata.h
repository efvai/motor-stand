#ifndef SIGNALDATA_H
#define SIGNALDATA_H


#include <QRect>
#include <QString>
#include <QList>

class dataTitle {
public:
    int index;
    QString channelName;
    QString name;
    QString status;
    bool enabled;
    dataTitle(int idx, const QString &chn, const QString &n) {
        index = idx;
        channelName = chn;
        name = n;
        status = "";
        enabled = true;
    }
    dataTitle(int idx, const QString &chn, const QString &n, bool en) {
        index = idx;
        channelName = chn;
        name = n;
        status = "";
        enabled = en;
    }



public:
    void setStatus(const QString &newStatus) {
        status = newStatus;
    }
    void setEnabled(bool en) {
        enabled = en;
    }
};

class PlotsList
{
public:
    static PlotsList& instance() {
        static PlotsList l;
        return l;
    }
    QList<dataTitle> plotsList = {
        dataTitle(0, "Ltr 11", "LTR 11", true),
        dataTitle(1, "Ltr 22, канал 1", "LTR 22", true),
        dataTitle(2, "Ltr 22, канал 2", "LTR 22", false),
        dataTitle(3, "Ltr 22, канал 3", "LTR 22", false),
        dataTitle(4, "Ltr 22, канал 4", "LTR 22", false),
        dataTitle(5, "Ltr 34, канал 1", "LTR 34", true),
        dataTitle(6, "Modbus (test)", "MODBUS", false)

    };
    void setStatus(int idx, const QString &newStatus) {
        plotsList[idx].setStatus(newStatus);
    }
    void setEnabled(int idx, bool en) {
        plotsList[idx].setEnabled(en);
    }
private:
    PlotsList() {}
    ~PlotsList() {}
    Q_DISABLE_COPY( PlotsList )
};

class SignalData
{
  public:
    static SignalData& instance(int idx);

    void append( const QPointF& pos );
    void clearStaleValues( double min );
    void clearValues();
    void save();
    void assignSaved();

    int size() const;
    int sizeSaved() const;
    QPointF value( int index ) const;
    float savedYValue( int index) const;

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
