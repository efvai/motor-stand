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
class ModbusSettingsDialog;
}
QT_END_NAMESPACE

enum WriteActions{
    MOTOR_ENABLED,
    MOTOR_DISABLED,
    TARGET_SET,
    ZERO_TARGET
};
// Каждые 0,1 сек. посылаем запрос на чтение одного бита.
// Если пришла 1 (выставляет МК при заполнении массива), то читаем 600 слов и выставляем 0
// МК опять заполняет массив данными
// 600 / 2 = 300 (float)
// 200 - Скорость, 400 - Токи
// 0,1 / 200 = 0.0005 с - Период
class ModbusSettingsDialog;

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
    float Uint16ToFloat(quint16 *src);
    quint16* floatTUint16(float src, quint16 *dst);

private slots:
    void onConnect();

    void onRead();
    void onReadReady();

    void onWrite(WriteActions action);
    void onReadWrite(WriteActions action);

private:

    QModbusReply *lastRequest = nullptr;
    QModbusClient *modbusDevice = nullptr;
    ModbusSettingsDialog *m_settingsDialog = nullptr;
};
#endif // MODBUSMASTER_H
