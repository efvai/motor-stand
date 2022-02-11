#ifndef MODBUSMASTER_H
#define MODBUSMASTER_H

#include <QWidget>
#include <QModbusDataUnit>
#include <QTimer>

QT_BEGIN_NAMESPACE
class QModbusClient;
class QModbusReply;

namespace Ui {
class ModbusMaster;
class SettingsDialog;
}
QT_END_NAMESPACE

enum WriteActions{
    MOTOR_ENABLED,
    MOTOR_DISABLED,
    TARGET_SET,
    ZERO_TARGET
};

class SettingsDialog;

class ModbusMaster : public QWidget
{
    Q_OBJECT

public:
    ModbusMaster(QWidget *parent = nullptr);
    ~ModbusMaster();
private:
    void initActions();
    QModbusDataUnit readRequest() const;
    QModbusDataUnit writeRequest(WriteActions action) const;

private:
    QTimer *timer = nullptr;
    int timerPeriod = 10;
    float counter = 0.0;
    float Uint16ToFloat(quint16 *src);
    quint16* floatTUint16(float src, quint16 *dst);

private slots:
    void onConnectButtonClicked();
    void onModbusStateChanged(int state);

    void onZeroButtonClicked();
    void onEnableBoxStateChanged(int state);
    void onWriteButtonClicked();

    void onRead();
    void onReadReady();

    void onWrite(WriteActions action);
    void onReadWrite(WriteActions action);
public slots:
    void startReading();
    void stopReading();
signals:
    void sendBlock(const float block);

private:
    Ui::ModbusMaster *ui;
    QModbusReply *lastRequest = nullptr;
    QModbusClient *modbusDevice = nullptr;
    SettingsDialog *m_settingsDialog = nullptr;
};
#endif // MODBUSMASTER_H
