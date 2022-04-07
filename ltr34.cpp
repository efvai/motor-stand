#include "ltr34.h"
#include "Plots/signaldata.h"

#include <QDebug>

Ltr34::Ltr34(QObject *parent) : QThread(parent)
{
    m_module = new TLTR34;
    m_ltr34_error = LTR34_Init(m_module);
    if (m_ltr34_error != LTR_OK) {
        setStatus(QString("Ошибка %1").arg(m_ltr34_error));
        LTR34_Close(m_module);
        return;
    }
    m_ltr34_error = LTR34_Open(m_module, m_param.addr, m_param.sport, m_param.serial, m_param.slot);
    if (m_ltr34_error != LTR_OK) {
        if (m_ltr34_error==LTR_WARNING_MODULE_IN_USE) {
            setStatus("Модуль уже открыт");
            m_ltr34_error = LTR_OK;
        }
        setStatus(QString("Ошибка %1").arg(m_ltr34_error));
        return;
    } else {
        getModuleInfo();
        configureModule();
    }
}

Ltr34::~Ltr34() {
    m_mutex.lock();
    m_abort = true;
    LTR34_Close(m_module);
    m_cond.wakeOne();
    m_mutex.unlock();
    wait();
}

void Ltr34::transaction(float request)
{
    req = request;
    const QMutexLocker locker(&m_mutex);
    if (!isRunning()) {
        start();
    }
    else {
        LTR34_DACStop(m_module);
        m_cond.wakeOne();
    }
}

void Ltr34::getModuleInfo()
{
    moduleInfo = QString("Название: %1\nСерийный номер: %2\nВерсия прошивки: %3\n")
            .arg(m_module->ModuleInfo.Name)
            .arg(m_module->ModuleInfo.Serial)
            .arg(m_module->ModuleInfo.FPGA_Version);
    emit sendInfo(moduleInfo);
}

void Ltr34::configureModule()
{
    m_module->ChannelQnt = 1;
    m_module->LChTbl[0]=LTR34_CreateLChannel(1,0);
    LTR34_FindDacFreqDivisor(31250, m_module->ChannelQnt, &m_module->FrequencyDivisor, NULL);
    m_module->UseClb=true; // Используем заводские калибровочные коэффициенты
    m_module->AcknowledgeType=false; // тип подтверждения - периодические
    m_module->ExternalStart=false; // внешний старт отключен
    m_module->RingMode=true; // включен режим автогенерации
    m_ltr34_error = LTR34_Reset(m_module);
    if (m_ltr34_error != LTR_OK) {
        setStatus(QString("Ошибка %1").arg(m_ltr34_error));
        LTR34_Close(m_module);
        return;
    }
    m_ltr34_error = LTR34_Config(m_module);
    if (m_ltr34_error != LTR_OK) {
        setStatus(QString("Ошибка %1").arg(m_ltr34_error));
        LTR34_Close(m_module);
        return;
    }
    else {
        setStatus("Готов");
    }
}

void Ltr34::setStatus(const QString &status)
{
    qDebug() << "Ltr34 status:" << status;
    PlotsList::instance().setStatus(5, status);
    emit sendStatus();
}

void Ltr34::run() {
    if (m_ltr34_error != LTR_OK) {
        QString err = QString("Ошибка %1").arg(m_ltr34_error);
        setStatus(err);
        emit error(err);
        return;
    }
    DWORD send = 0;
    DWORD size = 1;
    INT sizeSent = 0;
    while (m_ltr34_error == LTR_OK && !m_abort) {
        configureModule();
        m_ltr34_error = LTR34_ProcessData(m_module, &req, &send, size, true);
        if (m_ltr34_error != LTR_OK) {
            QString err = QString("Ошибка %1").arg(m_ltr34_error);
            setStatus(err);
            emit error(err);
        }
        sizeSent = LTR34_Send(m_module, &send, size, 4000);
        if (sizeSent != (INT) size) {
            if (sizeSent < 0) {
                QString err = QString("Ошибка %1").arg(sizeSent);
                setStatus(err);
                emit error(err);
            }
            else if (sizeSent < (INT) size) {
                QString err = "Ошибка отправки данных в модуль";
                setStatus(err);
                emit error(err);
            }
        }
        m_ltr34_error = LTR34_DACStart(m_module);
        if (m_ltr34_error != LTR_OK) {
            QString err = QString("Ошибка %1").arg(m_ltr34_error);
            setStatus(err);
            emit error(err);
        }
        else {
            setStatus(QString("Запущен"));
        }
        emit response();
        m_mutex.lock();
        m_cond.wait(&m_mutex);
        m_mutex.unlock();
    }
}


