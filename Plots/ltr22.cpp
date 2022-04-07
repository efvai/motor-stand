#include "ltr22.h"
#include "signaldata.h"
#include "Settings/LCardSettings/ltr22settingsdialog.h"
#include <QDebug>

Ltr22::Ltr22(QObject *parent)
    : QThread(parent)
{
    settings = new Ltr22Settings;
    m_module = new TLTR22();
    m_ltr22_error = 0;
    getParams(slot);
    initModule();
}

Ltr22::~Ltr22() {
    m_mutex.lock();
    m_abort = true;
    m_cond.wakeOne();
    m_mutex.unlock();
    wait();
}

void Ltr22::getParams(int slot) {
    m_param.slot = LTR_CC_CHNUM_MODULE1;
    m_param.serial = "";
    m_param.addr = LTRD_ADDR_DEFAULT;
    m_param.sport = LTRD_PORT_DEFAULT;
    if (slot > 0) {
        m_param.slot = slot;
    }

}

bool Ltr22::pause() const
{
    return m_pause;
}

void Ltr22::setPause(bool newPause)
{
    m_mutex.lock();
    m_pause = newPause;
    m_mutex.unlock();
}

void Ltr22::initModule() {

    //m_ltr22_error = LTR22_Close(m_module);
    LTR22_GetConfig(m_module);
    LTR22_Init(m_module);
    m_ltr22_error = LTR22_Open(m_module, m_param.addr, m_param.sport,
                     m_param.serial, m_param.slot);

    if (m_ltr22_error==LTR_WARNING_MODULE_IN_USE) {
        setStatus("Модуль уже открыт", 0, true);
        qDebug("ltr22 уже открыт");
        m_ltr22_error = LTR_OK;
    }
    if (m_ltr22_error != LTR_OK) {
        QString status = QString("Ошибка %1").arg(m_ltr22_error);
        setStatus(status, 0, true);
        return;
    }
    if ((m_ltr22_error = LTR22_IsOpened(m_module)) != LTR_OK) {
        QString status = QString("Ошибка %1").arg(m_ltr22_error);
        setStatus(status, 0, true);
        return;
    }
    else {
        setStatus("Готов", 0, true);
    }
    if ((m_ltr22_error=LTR22_GetModuleDescription(m_module)) == LTR_OK) {
        QString deviceName = reinterpret_cast<char*>(m_module->ModuleInfo.Description.DeviceName);
        QString serialNumber = reinterpret_cast<char*>(m_module->ModuleInfo.Description.SerialNumber);
        moduleInfo = QString("Название : %1\nСерийный номер   : %2\n")
                .arg(deviceName)
                .arg(serialNumber);
        emit sendInfo(moduleInfo);
        configureModule();
    }
}

void Ltr22::closeModule()
{
    m_ltr22_error = LTR22_Close(m_module);
    if (m_ltr22_error != LTR_OK) {
        QString status = QString("Ошибка %1").arg(m_ltr22_error);
        setStatus(status, 0, true);
    }
    else {
        QString status = QString("Закрыт");
        setStatus(status, 0, true);
    }
}

void Ltr22::configureModule()
{
    LTR22_FindAdcFreqParams(settings->adcFrequency, &m_module->Fdiv_rg,
                            &m_module->Adc384, &m_module->FreqDiscretizationIndex, NULL);
    LTR22_SetConfig(m_module);
    LTR22_SetADCChannel(m_module, 0, PlotsList::instance().plotsList.at(1).enabled);
    LTR22_SetADCChannel(m_module, 1, PlotsList::instance().plotsList.at(2).enabled);
    LTR22_SetADCChannel(m_module, 2, PlotsList::instance().plotsList.at(3).enabled);
    LTR22_SetADCChannel(m_module, 3, PlotsList::instance().plotsList.at(4).enabled);
    m_activeChannels = 0;
    for (int channel = 0; channel < 4; channel++) {
        int ch = channel + 1;
        if (PlotsList::instance().plotsList.at(ch).enabled) {
            if ((m_ltr22_error = LTR22_SetADCRange(m_module, channel, LTR22_ADC_RANGE_3)) != LTR_OK) {
                QString status = QString("Ошибка %1").arg(m_ltr22_error);
                setStatus(status, channel, false);
            }
            m_activeChannels++;
        }
    }
    LTR22_SwitchACDCState(m_module, settings->acdc);
    LTR22_SwitchMeasureADCZero(m_module, settings->zeroMeas);
}

void Ltr22::transaction()
{
    if (m_ltr22_error != LTR_OK) {
        return;
    }
    const QMutexLocker locker(&m_mutex);
    if (!isRunning()) {
        configureModule();
        start();
    }
    else
        m_cond.wakeOne();
}

void Ltr22::setStatus(const QString &status, int ch, bool all)
{
    qDebug() << status;
    if (all) {
        for (int i = 1; i < 5; i++) {
            PlotsList::instance().setStatus(i, status);
        }
    }
    else {
        PlotsList::instance().setStatus(ch+1, status);
    }
    emit sendStatus();
}

void Ltr22::run() {
    if (m_ltr22_error != LTR_OK) {
        QString status = QString("Ошибка %1").arg(m_ltr22_error);
        setStatus(status, 0, true);
        return;
    }
    DWORD receivedBlocks = 0;
    INT receivedDataCnt = RECV_BLOCK_CH_SIZE * m_activeChannels;
    DWORD *rbuf = new DWORD[receivedDataCnt]();
    double *data = new double[receivedDataCnt]();
    if ((rbuf == nullptr) || (data == nullptr)) {
        QString status = QString("Ошибка выделения памяти");
        setStatus(status, 0, true);
        m_ltr22_error = LTR_ERROR_MEMORY_ALLOC;
        return;
    }
    while (m_ltr22_error==LTR_OK && !m_abort) {
        if (m_pause) {
            configureModule();
            m_ltr22_error = LTR22_StartADC(m_module, false);
            if (m_ltr22_error != LTR_OK) {
                QString status = QString("Ошибка %1").arg(m_ltr22_error);
                setStatus(status, 0, true);
                delete[] rbuf;
                delete[] data;
                return;
            }
            m_pause = false;
            time = 0.0;
            receivedBlocks = 0;
            QString status = QString("Сбор данных запущен");
            setStatus(status, 0, true);
        }
        INT recvd;
        DWORD tout = RECV_TOUT;
        recvd = LTR22_Recv(m_module, rbuf, NULL, receivedDataCnt, tout);
        if (recvd < 0) {
            m_ltr22_error = recvd;
            QString status = QString("Ошибка приема данных. Ошибка %1").arg(m_ltr22_error);
            setStatus(status, 0, true);
        } else if (recvd!=receivedDataCnt) {
            QString status = QString("Принято недостаточно данных. Запрашивали %1, приняли %2")
                    .arg(receivedDataCnt).arg(recvd);
            setStatus(status, 0, true);
            m_ltr22_error = LTR_ERROR_RECV_INSUFFICIENT_DATA;
        } else {
            m_ltr22_error = LTR22_ProcessDataEx(m_module, rbuf, data, &recvd, LTR22_PROC_FLAG_CALIBR | LTR22_PROC_FLAG_VOLT, NULL, NULL);
            if (m_ltr22_error!=LTR_OK) {
                QString status = QString("Ошибка %1").arg(m_ltr22_error);
                setStatus(status, 0, true);
            } else {
                receivedBlocks++;
                for (INT i = 0; i < receivedDataCnt - m_activeChannels - 1; i+=m_activeChannels) {
                    if (PlotsList::instance().plotsList.at(1).enabled)
                        SignalData::instance(1).append(QPointF(time, data[i]));
                    if (PlotsList::instance().plotsList.at(2).enabled)
                        SignalData::instance(2).append(QPointF(time, data[i + 1]));
                    if (PlotsList::instance().plotsList.at(3).enabled)
                        SignalData::instance(3).append(QPointF(time, data[i + 2]));
                    if (PlotsList::instance().plotsList.at(4).enabled)
                        SignalData::instance(4).append(QPointF(time, data[i + 3]));
                    time += 1.0/settings->adcFrequency;
                }
            }
        }
        if (m_pause) {
            m_mutex.lock();
            QString status = QString("Сбор данных остановлен");
            setStatus(status, 0, true);
            LTR22_StopADC(m_module);
            LTR22_ClearBuffer(m_module, true);
            m_cond.wait(&m_mutex);
            m_mutex.unlock();
        }
    }
    delete[] rbuf;
    delete[] data;
}

void Ltr22::setSettings(Ltr22Settings newSettings)
{
    settings->adcFrequency = newSettings.adcFrequency;
    settings->acdc = newSettings.acdc;
    settings->zeroMeas = newSettings.zeroMeas;
    if (m_ltr22_error == LTR_OK)
        configureModule();
}

const QString &Ltr22::getModuleInfo() const
{
    return moduleInfo;
}
