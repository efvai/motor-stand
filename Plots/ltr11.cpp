#include "ltr11.h"
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <QDebug>
#include <QProcess>
#include "signaldata.h"
#include <QPointF>

Ltr11::Ltr11(QObject *parent)
    : QThread(parent)
{
  //  QProcess::startDetached("./ltrd/ltrd.exe", QStringList() << "--start");
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
        QString status = QString("Не удалось установить связь с модулем. Ошибка %1")
                .arg(m_ltr11_error);
        setStatus(status);
    } else {
        m_ltr11_error = LTR11_GetConfig(&hltr11);
        if (m_ltr11_error != LTR_OK) {
            QString status = QString("Не удалось прочитать информацию о модуле.\n Ошибка %1 (%2)")
                    .arg(m_ltr11_error).arg(LTR11_GetErrorString(m_ltr11_error));
            setStatus(status);
        } else {  /* конфигурация получена успешно */
        /* вывод информации о модуле */
        qDebug( "Информация о модуле:\n"
                "  Название модуля  : %s\n"
                "  Серийный номер   : %s\n"
                "  Версия прошивки  : %u.%u\n",
                   hltr11.ModuleInfo.Name, hltr11.ModuleInfo.Serial,
                  (hltr11.ModuleInfo.Ver >> 8) & 0xFF, hltr11.ModuleInfo.Ver & 0xFF);
        configure();
        }
    }
}

void Ltr11::configure() {
    /* задание параметров работы модуля */
    /* режим старта сбора данных - внутренний */
    hltr11.StartADCMode = LTR11_STARTADCMODE_INT;
    /* режим синхронизации АПЦ - внутренний */
    hltr11.InpMode      = LTR11_INPMODE_INT;
    /* количество логических каналов - 4 */
    hltr11.LChQnt       = 1;
    /* таблица управления логическими каналами */
    /* диапазон - 10В, режим - 16-канальный, физический канал - 1 */
    hltr11.LChTbl[0] = LTR11_CreateLChannel(0, LTR11_CHMODE_DIFF, LTR11_CHRANGE_10000MV);
    /* диапазон - 2.5В, режим - измерение собственного нуля, физический канал - 2 */
    //hltr11.LChTbl[1] = LTR11_CreateLChannel(1, LTR11_CHMODE_ZERO, LTR11_CHRANGE_2500MV);
    /* диапазон - 0.6В, режим - 32-канальный (каналы 1..16), физический канал - 3 */
    //hltr11.LChTbl[2] = LTR11_CreateLChannel(2, LTR11_CHMODE_COMM, LTR11_CHRANGE_625MV);
    /* диапазон - 0.15В, режим - 32-канальный (каналы 17..32), физический канал - 25 */
    //hltr11.LChTbl[3] = LTR11_CreateLChannel(24, LTR11_CHMODE_COMM, LTR11_CHRANGE_156MV);
    /* режим сбора данных */
    hltr11.ADCMode = LTR11_ADCMODE_ACQ;
    /* частота дискретизации - 400 кГц. Подбираем нужные делители */
    LTR11_FindAdcFreqParams(ADC_FREQ, &hltr11.ADCRate.prescaler,
                            &hltr11.ADCRate.divider, NULL);

    /* передаем настройки в модуль */
    m_ltr11_error = LTR11_SetADC(&hltr11);
    if (m_ltr11_error != LTR_OK) {
        QString status = QString("Не удалось установить настройки модуля. Ошибка %1 (%2)")
                .arg(m_ltr11_error).arg(LTR11_GetErrorString(m_ltr11_error));
        setStatus(status);
    }
    QString status = QString("Готов");
    setStatus(status);
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
    //configure();
    if (!isRunning()) {
        configure();
        start();
    }
    else
        m_cond.wakeOne();
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
            //configure();
            /* запуск сбора данных */
            m_ltr11_error = LTR11_Start(&hltr11);
            if (m_ltr11_error != LTR_OK) {
                QString status = QString("Не удалось запустить сбор данных. Ошибка %1 (%2)")
                        .arg(m_ltr11_error).arg(LTR11_GetErrorString(m_ltr11_error));
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
        /* в таймауте учитываем время выполнения самого преобразования*/
        DWORD tout = RECV_TOUT + (DWORD)(RECV_BLOCK_CH_SIZE/hltr11.ChRate + 1);
        /* получение данных от LTR11 */
        recvd = LTR11_Recv(&hltr11, rbuf, NULL, receivedDataCnt, tout);
        /* Значение меньше нуля соответствуют коду ошибки */
        if (recvd < 0) {
            m_ltr11_error = recvd;
            QString status = QString("Ошибка приема данных. Ошибка %1 (%2)")
                    .arg(m_ltr11_error).arg(LTR11_GetErrorString(m_ltr11_error));
            setStatus(status);
        } else if (recvd!=receivedDataCnt) {
            QString status = QString("Принято недостаточно данных. Запрашивали %1, приняли %2")
                    .arg(receivedDataCnt).arg(recvd);
            setStatus(status);
            m_ltr11_error = LTR_ERROR_RECV_INSUFFICIENT_DATA;
        } else {
            /* сохранение принятых и обработанных данных в буфере */
            m_ltr11_error = LTR11_ProcessData(&hltr11, rbuf, data, &recvd, TRUE, TRUE);
            if (m_ltr11_error!=LTR_OK) {
                QString status = QString("Ошибка обработки данных. Ошибка %1, %2")
                        .arg(m_ltr11_error).arg(LTR11_GetErrorString(m_ltr11_error));
                setStatus(status);
            } else {
                /* Принимаем данные */
                receivedBlocks++;
                //qDebug("Блок %4d: ", receivedBlocks);
                for (INT i = 0; i < receivedDataCnt; i++) {
                    SignalData::instance(0).append(QPointF(time, data[i]));
                    time += 1.0/ADC_FREQ;
                }
                //m_block.insert(m_block.end(), &data[0], &data[receivedDataCnt]);
                //emit sendBlock(m_block);
                //m_block.clear();
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
        /* закрытие канала связи с модулем */
        LTR11_Close(&hltr11);

    }
   // QProcess::startDetached("./ltrd/ltrd.exe", QStringList() << "--stop");
}


void Ltr11::stopProcess() {
    m_mutex.lock();
    m_abort = true;
    m_mutex.unlock();
}
