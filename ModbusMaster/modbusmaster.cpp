#include "modbusmaster.h"
#include "ui_modbusmaster.h"
#include "Settings/modbussettingsdialog.h"

#include <QDebug>
#include <QModbusRtuSerialMaster>
#include <QStandardItemModel>
#include <QStatusBar>
#include <QUrl>
#include <cstring>

ModbusMaster::ModbusMaster(QWidget *parent)
    : QWidget(parent)
{

    m_settingsDialog = new ModbusSettingsDialog(this);
    timer = new QTimer(this);

    initActions();
    modbusDevice = new QModbusRtuSerialMaster(this);
    connect(modbusDevice, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
        qDebug() << modbusDevice->errorString() << "\n";
    });
    if (!modbusDevice) {
        qDebug() << tr("ModbusMaster() -> Could not create Modbus master.") << "\n";
    } else {
        //connect(modbusDevice, &QModbusClient::stateChanged,
        //        this, &ModbusMaster::onModbusStateChanged);
    }

    connect(timer, &QTimer::timeout, this, &ModbusMaster::onRead);


}

ModbusMaster::~ModbusMaster()
{
    if (modbusDevice) {
        modbusDevice->disconnectDevice();
    }
    delete modbusDevice;
}

void ModbusMaster::initActions() {

}

void ModbusMaster::onConnect() {
    if (!modbusDevice)
        return;

    if (modbusDevice->state() != QModbusDevice::ConnectedState) {
        modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter,
            m_settingsDialog->settings().port);
        modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter,
            m_settingsDialog->settings().parity);
        modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,
            m_settingsDialog->settings().baud);
        modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,
            m_settingsDialog->settings().dataBits);
        modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,
            m_settingsDialog->settings().stopBits);
        modbusDevice->setTimeout(m_settingsDialog->settings().responseTime);
        modbusDevice->setNumberOfRetries(m_settingsDialog->settings().numberOfRetries);
        if (!modbusDevice->connectDevice()) {
            qDebug() << tr("onConnectButtonClicked() -> Connect failed: ") + modbusDevice->errorString() << "\n";
        }
    } else {
        modbusDevice->disconnectDevice();
    }

}

void ModbusMaster::onRead()
{
    if (!modbusDevice)
        return;

    if (auto *reply = modbusDevice->sendReadRequest(readRequest(), m_settingsDialog->settings().serverAddress)) {
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, &ModbusMaster::onReadReady);
        else
            delete reply; // broadcast replies return immediately
    } else {
        qDebug() << tr("onRead() -> Read error: ") + modbusDevice->errorString() << "\n";
    }
}

void ModbusMaster::onReadReady() {
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
//      for (int i = 0, total = int(unit.valueCount()); i < total; ++i) {
//            const QString entry = tr("Address: %1, Value: %2").arg(unit.startAddress() + i)
//                                     .arg(QString::number(unit.value(i),
//                                          unit.registerType() <= QModbusDataUnit::Coils ? 10 : 16));
//            qDebug() << entry << "\n";

//      }
        quint16 reg[2] = {unit.value(1), unit.value(0)};
        float value = Uint16ToFloat(reg);
       // ui->velLcd->display(value);
    } else if (reply->error() == QModbusDevice::ProtocolError) {
        qDebug() << tr("onReadReady() -> Read response error: %1 (Mobus exception: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->rawResult().exceptionCode(), -1, 16) << "\n";
    } else {
        qDebug() << tr("onReadReady() -> Read response error: %1 (code: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->error(), -1, 16) << "\n";
    }

    reply->deleteLater();
}

void ModbusMaster::onWrite(WriteActions action) {
    if (!modbusDevice)
        return;
    QModbusDataUnit writeUnit = writeRequest(action);
    if (!writeUnit.isValid()) {
        qDebug() << "onWrite(): write unit isnt valid! \n";
        return;
    }
    switch (action) {
    case WriteActions::MOTOR_DISABLED:
        writeUnit.setValue(0, 0);
        break;
    case WriteActions::MOTOR_ENABLED:
        writeUnit.setValue(0, 1);
        break;
    case WriteActions::ZERO_TARGET:
        writeUnit.setValue(0, 0);
        writeUnit.setValue(1, 0);
        break;
    case WriteActions::TARGET_SET:
       // float target = ui->targetSpinner->value();
        quint16 reg[2] = {0, 0};
        //floatTUint16(target, reg);
        writeUnit.setValue(0, reg[0]);
        writeUnit.setValue(0, reg[1]);
        break;
    }
    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, m_settingsDialog->settings().serverAddress)) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [reply]() {
                if (reply->error() == QModbusDevice::ProtocolError) {
                    qDebug() << tr("Write response error: %1 (Mobus exception: 0x%2)")
                                .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16)
                             << "\n";
                } else if (reply->error() != QModbusDevice::NoError) {
                    qDebug() << tr("Write response error: %1 (code: 0x%2)").
                                arg(reply->errorString()).arg(reply->error(), -1, 16) << "\n";
                }
                reply->deleteLater();
            });
        } else {
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else {
        qDebug() << tr("Write error: ") + modbusDevice->errorString() << "\n";
    }
}

void ModbusMaster::onReadWrite(WriteActions action)
{
    if (!modbusDevice)
        return;
    QModbusDataUnit writeUnit = writeRequest(action);
    if (!writeUnit.isValid()) {
        qDebug() << "onReadWrite(): write unit isnt valid! \n";
        return;
    }
    switch (action) {
    case WriteActions::MOTOR_DISABLED:
        writeUnit.setValue(0, 0);
        break;
    case WriteActions::MOTOR_ENABLED:
        writeUnit.setValue(0, 1);
        break;
    case WriteActions::ZERO_TARGET:
        writeUnit.setValue(0, 0);
        writeUnit.setValue(1, 0);
        break;
    case WriteActions::TARGET_SET:
     //   float target = ui->targetSpinner->value();
        quint16 reg[2] = {0, 0};
        //floatTUint16(target, reg);
        writeUnit.setValue(0, reg[0]);
        writeUnit.setValue(0, reg[1]);
        break;
    }
    if (auto *reply = modbusDevice->sendReadWriteRequest(readRequest(), writeUnit,
        m_settingsDialog->settings().serverAddress)) {
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, &ModbusMaster::onReadReady);
        else
            delete reply; // broadcast replies return immediately
    } else {
        qDebug() << tr("Read error: ") + modbusDevice->errorString() << "\n";
    }
}

QModbusDataUnit ModbusMaster::readRequest() const {
    const auto type = QModbusDataUnit::InputRegisters;
    int startAddress = 0;
    quint16 numberofEntries = 2;
    return QModbusDataUnit(type, startAddress, numberofEntries);
}

QModbusDataUnit ModbusMaster::writeRequest(WriteActions action) const
{
    QModbusDataUnit::RegisterType type = QModbusDataUnit::Invalid;
    if ((action == WriteActions::MOTOR_ENABLED) || (action == WriteActions::MOTOR_DISABLED))
    {
       type = QModbusDataUnit::Coils;
       int startAddress = 0;
       quint16 numberofEntries = 1;
       return QModbusDataUnit(type, startAddress, numberofEntries);
    } else if (action == WriteActions::ZERO_TARGET || action == WriteActions::TARGET_SET) {
        type = QModbusDataUnit::HoldingRegisters;
        int startAddress = 0;
        quint16 numberofEntries = 2;
        return QModbusDataUnit(type, startAddress, numberofEntries);
    } else {
        return QModbusDataUnit(type, 0, 0);
    }


}

float ModbusMaster::Uint16ToFloat(quint16 *src)
{
    float dst = 0.0f;
    Q_ASSERT(sizeof(dst) == sizeof(src));
    std::memcpy(&dst, src, sizeof(float));
    qDebug() << "float: " << dst << "\n";
    return dst;
}

quint16* ModbusMaster::floatTUint16(float src, quint16* dst) {
    Q_ASSERT(sizeof(src) == sizeof(dst));
    std::memcpy(dst, &src, sizeof(float));
    return dst;
}

