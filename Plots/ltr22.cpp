#include "ltr22.h"
#include "signaldata.h"
#include <QDebug>

Ltr22::Ltr22(QObject *parent)
    : QThread(parent)
{
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
    m_ltr22_error = LTR22_Close(m_module);
    m_ltr22_error = LTR22_Open(m_module, m_param.addr, m_param.sport,
                     m_param.serial, m_param.slot);
    if (m_ltr22_error==LTR_WARNING_MODULE_IN_USE) {
        setStatus("Модуль уже открыт", 0, true);
        m_ltr22_error = LTR_OK;
    }
    if (m_ltr22_error != LTR_OK) {
        QString status = QString("Не удалось получить доступ. Ошибка %1").arg(m_ltr22_error);
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
    if ((m_ltr22_error=LTR22_GetModuleDescription(m_module)) == LTR_OK)
        qDebug( "Информация о модуле:\n"
            "  Название модуля  : %s\n"
            "  Серийный номер   : %s\n"
            "  Версия прошивки  : %u.%u\n",
               m_module->ModuleInfo.Description.DeviceName, m_module->ModuleInfo.Description.SerialNumber,
              (m_module->ModuleInfo.Description.VerPLD >> 8) & 0xFF, m_module->ModuleInfo.Description.VerPLD & 0xFF);
    configureModule();

}

void Ltr22::closeModule()
{
    m_ltr22_error = LTR22_Close(m_module);
    if (m_ltr22_error != LTR_OK) {
        QString status = QString("Не удалось закрыть. Ошибка %1").arg(m_ltr22_error);
        setStatus(status, 0, true);
    }
    else {
        QString status = QString("Закрыт");
        setStatus(status, 0, true);
    }
}

void Ltr22::configureModule()
{
    m_ltr22_error = LTR22_SetFreq(m_module, true, 15);
    LTR22_SetADCChannel(m_module, 1, true);
    LTR22_SetADCChannel(m_module, 2, false);
    LTR22_SetADCChannel(m_module, 3, false);
    LTR22_SetADCChannel(m_module, 0, true);
    if ((m_ltr22_error = LTR22_SetADCRange(m_module, 0, LTR22_ADC_RANGE_3)) != LTR_OK) {
        QString status = QString("Ошибка настройки диапазона. Ошибка %1").arg(m_ltr22_error);
        setStatus(status, 0, false);
    }
    if ((m_ltr22_error = LTR22_SetADCRange(m_module, 1, LTR22_ADC_RANGE_3)) != LTR_OK) {
        QString status = QString("Ошибка настройки диапазона. Ошибка %1").arg(m_ltr22_error);
        setStatus(status, 1, false);
    }
//    if ((m_ltr22_error=LTR22_SetADCRange(m_module, 2, LTR22_ADC_RANGE_10))!=LTR_OK) {
//        QString status = QString("Ошибка настройки диапазона. Ошибка %1").arg(m_ltr22_error);
//        setStatus(status, 2, false);
//    }
//    if ((m_ltr22_error=LTR22_SetADCRange(m_module, 3, LTR22_ADC_RANGE_10))!=LTR_OK) {
//        QString status = QString("Ошибка настройки диапазона. Ошибка %1").arg(m_ltr22_error);
//        setStatus(status, 3, false);
//    }
    if ((m_ltr22_error=LTR22_GetConfig(m_module))!=LTR_OK) {
        QString status = QString("Ошибка получения конфигурации. Ошибка %1").arg(m_ltr22_error);
        setStatus(status, 0, true);
    }
//    for (int i = 0; i < 4; i++) {
//        qDebug() << m_module->ChannelEnabled[i] << " Channel";
//        qDebug() << m_module->ADCChannelRange[i] << " Range";
//    }
    //LTR22_GetCalibrovka(m_module);

    //LTR22_SwitchACDCState(m_module, false);
    //LTR22_SwitchMeasureADCZero(m_module, true);
}

void Ltr22::transaction()
{
    bool en = PlotsList::instance().plotsList.at(1).enabled |
            PlotsList::instance().plotsList.at(2).enabled |
            PlotsList::instance().plotsList.at(3).enabled |
            PlotsList::instance().plotsList.at(4).enabled;
    if (!en) {
        return;
    }
    const QMutexLocker locker(&m_mutex);
    //configureModule();
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
    INT receivedDataCnt = RECV_BLOCK_CH_SIZE * channels;
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
            //configureModule();
            m_ltr22_error = LTR22_StartADC(m_module, false);
            if (m_ltr22_error != LTR_OK) {
                QString status = QString("Не удалось запустить сбор данных. Ошибка %1").arg(m_ltr22_error);
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
        /* в таймауте учитываем время выполнения самого преобразования*/
        DWORD tout = RECV_TOUT;
        /* получение данных от LTR */
        recvd = LTR22_Recv(m_module, rbuf, NULL, receivedDataCnt, tout);
        /* Значение меньше нуля соответствуют коду ошибки */
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
            /* сохранение принятых и обработанных данных в буфере */
            m_ltr22_error = LTR22_ProcessData(m_module, rbuf, data, recvd, TRUE, FALSE, NULL);

            //qDebug() << data[0];
            if (m_ltr22_error!=LTR_OK) {
                QString status = QString("Ошибка обработки данных. Ошибка %1").arg(m_ltr22_error);
                setStatus(status, 0, true);
            } else {
                /* Принимаем данные */
                receivedBlocks++;
                for (INT i = 0; i < receivedDataCnt/2; i++) {
                    data[i] = (data[i] / 32768) * 3;
                    data[i + receivedDataCnt/2] = (data[i+receivedDataCnt/2] / 32768) * 3;
                    SignalData::instance(1).append(QPointF(time, data[i]));
                    SignalData::instance(2).append(QPointF(time, data[i+receivedDataCnt/2]));
                    time += 1.0/3472.0;
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
