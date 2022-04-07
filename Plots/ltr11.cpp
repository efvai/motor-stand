#include "ltr11.h"
#include <QDebug>
#include "signaldata.h"
#include <QPointF>
#include "Settings/LCardSettings/ltr11settingsdialog.h"

Ltr11::Ltr11(QObject *parent)
    : QThread(parent)
{
  //  QProcess::startDetached("./ltrd/ltrd.exe", QStringList() << "--start");
    settings = new Ltr11Settings;
    m_ltr11_error = 0;
    getParams(slot);
    if (!m_ltr11_error) {
        init();
    }
}

Ltr11::~Ltr11() {
    m_mutex.lock();
    m_abort = true;
    m_cond.wakeOne();
    m_mutex.unlock();
    wait();
}

void Ltr11::setSlot(int slot) {
    if (slot > 0) {
        this->slot = slot;
    } else {
        this->slot = -1;
    }
}

void Ltr11::getParams(int slot) {
    param.slot = LTR_CC_CHNUM_MODULE1;
    param.serial = "";
    param.addr = LTRD_ADDR_DEFAULT;
    if (slot > 0) {
        param.slot = slot;
    }

}

void Ltr11::init() {
    LTR11_Init(&hltr11);
    m_ltr11_error = LTR11_Open(&hltr11, param.addr, LTRD_PORT_DEFAULT, param.serial, param.slot);
    if (m_ltr11_error != LTR_OK) {
        QString status = QString("Ошибка %1")
                .arg(m_ltr11_error);
        setStatus(status);
    } else {
        m_ltr11_error = LTR11_GetConfig(&hltr11);
        if (m_ltr11_error != LTR_OK) {
            QString status = QString("Ошибка %1")
                    .arg(m_ltr11_error);
            setStatus(status);
        } else {
            moduleInfo = QString("Название : %1\nСерийный номер   : %2\nВерсия прошивки  : %3.%4\n")
                    .arg(hltr11.ModuleInfo.Name)
                    .arg(hltr11.ModuleInfo.Serial)
                    .arg((hltr11.ModuleInfo.Ver >> 8) & 0xFF)
                    .arg(hltr11.ModuleInfo.Ver & 0xFF);
            emit sendInfo(moduleInfo);
            configure();
        }
    }
}

void Ltr11::configure() {
    hltr11.StartADCMode = LTR11_STARTADCMODE_INT;
    hltr11.InpMode      = LTR11_INPMODE_INT;
    hltr11.LChQnt       = 1;
    hltr11.LChTbl[0] = LTR11_CreateLChannel(0, settings->type, LTR11_CHRANGE_10000MV);
    hltr11.ADCMode = LTR11_ADCMODE_ACQ;
    LTR11_FindAdcFreqParams(settings->adcFrequency, &hltr11.ADCRate.prescaler,
                            &hltr11.ADCRate.divider, NULL);
    m_ltr11_error = LTR11_SetADC(&hltr11);
    if (m_ltr11_error != LTR_OK) {
        QString status = QString("Ошибка %1")
                .arg(m_ltr11_error);
        setStatus(status);
    } else {
        QString status = QString("Готов");
        setStatus(status);
    }
}

void Ltr11::setStatus(const QString &status)
{
    qDebug() << status;
    PlotsList::instance().setStatus(0, status);
    emit sendStatus();
}

void Ltr11::transaction()
{
    if (!PlotsList::instance().plotsList.at(0).enabled) {
        return;
    }
    const QMutexLocker locker(&m_mutex);
    if (!isRunning()) {
        configure();
        start();
    }
    else
        m_cond.wakeOne();
}

const QString &Ltr11::getModuleInfo() const
{
    return moduleInfo;
}

void Ltr11::setSettings(Ltr11Settings newSettings)
{
    settings->type = newSettings.type;
    settings->adcFrequency = newSettings.adcFrequency;
    if (m_ltr11_error == LTR_OK)
        configure();
}

bool Ltr11::pause() const
{
    return m_pause;
}

void Ltr11::setPause(bool newPause)
{
    m_mutex.lock();
    m_pause = newPause;
    m_mutex.unlock();
}

void Ltr11::run() {
    if (m_ltr11_error != LTR_OK) {
        QString status = QString("Ошибка %1").arg(m_ltr11_error);
        setStatus(status);
        return;
    }
    DWORD receivedBlocks = 0;
    INT receivedDataCnt = RECV_BLOCK_CH_SIZE * hltr11.LChQnt;
    DWORD *rbuf = new DWORD[receivedDataCnt]();
    double *data = new double[receivedDataCnt]();
    if ((rbuf == nullptr) || (data == nullptr)) {
        m_ltr11_error = LTR_ERROR_MEMORY_ALLOC;
        QString status = QString("Ошибка выделения памяти!");
        setStatus(status);
        return;
    }

    while (m_ltr11_error==LTR_OK && !m_abort) {
        if (m_pause) {
            configure();
            m_ltr11_error = LTR11_Start(&hltr11);
            if (m_ltr11_error != LTR_OK) {
                QString status = QString("Ошибка %1")
                        .arg(m_ltr11_error);
                setStatus(status);
                delete[] rbuf;
                delete[] data;
                return;
            }
            m_pause = false;
            time = 0.0;
            receivedBlocks = 0;
            QString status = QString("Сбор данных запущен");
            setStatus(status);
        }
        INT recvd;
        DWORD tout = RECV_TOUT + (DWORD)(RECV_BLOCK_CH_SIZE/hltr11.ChRate + 1);
        recvd = LTR11_Recv(&hltr11, rbuf, NULL, receivedDataCnt, tout);
        if (recvd < 0) {
            m_ltr11_error = recvd;
            QString status = QString("Ошибка %1")
                    .arg(m_ltr11_error);
            setStatus(status);
        } else if (recvd!=receivedDataCnt) {
            QString status = QString("Принято недостаточно данных. Запрашивали %1, приняли %2")
                    .arg(receivedDataCnt).arg(recvd);
            setStatus(status);
            m_ltr11_error = LTR_ERROR_RECV_INSUFFICIENT_DATA;
        } else {
            m_ltr11_error = LTR11_ProcessData(&hltr11, rbuf, data, &recvd, TRUE, TRUE);
            if (m_ltr11_error!=LTR_OK) {
                QString status = QString("Ошибка %1")
                        .arg(m_ltr11_error);
                setStatus(status);
            } else {
                receivedBlocks++;
                for (INT i = 0; i < receivedDataCnt; i++) {
                    SignalData::instance(0).append(QPointF(time, data[i]));
                    time += 1.0/settings->adcFrequency;
                }
            }
        }
        if (m_pause) {
            m_mutex.lock();
            QString status = QString("Сбор данных остановлен");
            setStatus(status);
            LTR11_Stop(&hltr11);
            m_cond.wait(&m_mutex);

            m_mutex.unlock();
        }
    }
    delete[] rbuf;
    delete[] data;
}

void Ltr11::closeLtr() {
    if (LTR11_IsOpened(&hltr11) == LTR_OK) {
        LTR11_Stop(&hltr11);
        LTR11_Close(&hltr11);
    }
   // QProcess::startDetached("./ltrd/ltrd.exe", QStringList() << "--stop");
}


void Ltr11::stopProcess() {
    m_mutex.lock();
    m_abort = true;
    m_mutex.unlock();
}

