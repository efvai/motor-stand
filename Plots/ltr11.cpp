#include "ltr11.h"
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <QDebug>
#include <QProcess>

Ltr11::Ltr11(QObject *parent)
    : QThread(parent)
{
  //  QProcess::startDetached("./ltrd/ltrd.exe", QStringList() << "--start");
    m_abort = false;
    err = 0;
    getParams(slot);
    if (!err) {
        init();
    }
}

Ltr11::~Ltr11() {
    mutex.lock();
    m_abort = true;
    mutex.unlock();
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
    err = LTR11_Open(&hltr11, param.addr, LTRD_PORT_DEFAULT, param.serial, param.slot);
    if (err != LTR_OK) {
        qCritical("Не удалось установить связь с модулем. Ошибка %d (%s)\n",
                err, LTR11_GetErrorString(err));
    } else {
        err = LTR11_GetConfig(&hltr11);
        if (err != LTR_OK) {
            qCritical("Не удалось прочитать информацию о модуле. Ошибка %d: %s\n", err,
                    LTR11_GetErrorString(err));
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
    LTR11_FindAdcFreqParams(ACD_FREQ, &hltr11.ADCRate.prescaler,
                            &hltr11.ADCRate.divider, NULL);

    /* передаем настройки в модуль */
    err = LTR11_SetADC(&hltr11);
    if (err != LTR_OK) {
        qCritical("Не удалось установить настройки модуля. Ошибка %d: %s\n", err,
                LTR11_GetErrorString(err));
    }
}

void Ltr11::processReceive()
{
    m_abort = false;
    start();
}

void Ltr11::receive() {
    int f_out = 0;
    if (err == LTR_OK) {
        DWORD recvd_blocks=0;
        INT recv_data_cnt = RECV_BLOCK_CH_SIZE * hltr11.LChQnt;
        DWORD  *rbuf = (DWORD*)malloc(recv_data_cnt*sizeof(rbuf[0]));
        double *data = (double *)malloc(recv_data_cnt*sizeof(data[0]));
        if ((rbuf==NULL) || (data==NULL)) {
            qCritical("Ошибка выделения памяти!\n");
            err = LTR_ERROR_MEMORY_ALLOC;
        } else {
            /* запуск сбора данных */
            err = LTR11_Start(&hltr11);
            if (err != LTR_OK) {
                qCritical("Не удалось запустить сбор данных. Ошибка %d: %s\n", err,
                        LTR11_GetErrorString(err));
            } else {
                INT stop_err = 0;
                qDebug("Сбор данных запущен. Для останова нажмите %s\n",
                       "любую клавишу");

                while (!f_out && (err==LTR_OK)) {
                    INT recvd;
                    /* в таймауте учитываем время выполнения самого преобразования*/
                    DWORD tout = RECV_TOUT + (DWORD)(RECV_BLOCK_CH_SIZE/hltr11.ChRate + 1);
                    /* получение данных от LTR11 */
                    recvd = LTR11_Recv(&hltr11, rbuf, NULL, recv_data_cnt, tout);
                    /* Значение меньше нуля соответствуют коду ошибки */
                    if (recvd<0) {
                        err = recvd;
                        qCritical("Ошибка приема данных. Ошибка %d:%s\n",
                                err, LTR11_GetErrorString(err));
                    } else if (recvd!=recv_data_cnt) {
                        qCritical("Принято недостаточно данных. Запрашивали %d, приняли %d\n",
                                recv_data_cnt, recvd);
                        err = LTR_ERROR_RECV_INSUFFICIENT_DATA;
                    } else {
                        /* сохранение принятых и обработанных данных в буфере */
                        err = LTR11_ProcessData(&hltr11, rbuf, data, &recvd, TRUE, TRUE);
                        if (err!=LTR_OK) {
                            qCritical("Ошибка обработки данных. Ошибка %d:%s\n",
                                    err, LTR11_GetErrorString(err));
                        } else {
                            INT ch;
                            recvd_blocks++;
                            /* выводим по первому слову на канал */
                            qDebug("Блок %4d: ", recvd_blocks);

                            for (ch=0; ch < hltr11.LChQnt; ch++) {
                                qDebug("%8.2f", data[ch]);

                                if (ch==(hltr11.LChQnt-1)) {
                                    qDebug("\n");
                                } else {
                                    qDebug(",  ");
                                }
                            }

                        }
                    }
                    if (err == LTR_OK) {
                       // if (recvd_blocks > 10)
                        //    f_out = 1;
                    }
                }
                /* останавливаем сбор данных */
                stop_err = LTR11_Stop(&hltr11);
                if (stop_err!=LTR_OK) {
                    qCritical("Не удалось остановить сбор данных. Ошибка %d: %s\n", err,
                        LTR11_GetErrorString(stop_err));
                    if (err == LTR_OK)
                        err = stop_err;
                } else {
                    qDebug("Сбор остановлен успешно!\n");
                }
            }

            free(rbuf);
            free(data);
        }
    }
    closeLtr();
}

void Ltr11::run() {
    if (err != LTR_OK) {
        qDebug() << "Run error\n";
        return;
    }
    DWORD receivedBlocks = 0;
    INT receivedDataCnt = RECV_BLOCK_CH_SIZE * hltr11.LChQnt;
    DWORD *rbuf = new DWORD[receivedDataCnt]();
    double *data = new double[receivedDataCnt]();
    if ((rbuf == nullptr) || (data == nullptr)) {
        qCritical("Ошибка выделения памяти!\n");
        err = LTR_ERROR_MEMORY_ALLOC;
        return;
    }
    /* запуск сбора данных */
    err = LTR11_Start(&hltr11);
    if (err != LTR_OK) {
        qCritical("Не удалось запустить сбор данных. Ошибка %d: %s\n", err,
                LTR11_GetErrorString(err));
        delete[] rbuf;
        delete[] data;
        return;
    }
    //INT stop_err = 0;
    qDebug("Сбор данных запущен.");
    while (err==LTR_OK) {
        INT recvd;
        /* в таймауте учитываем время выполнения самого преобразования*/
        DWORD tout = RECV_TOUT + (DWORD)(RECV_BLOCK_CH_SIZE/hltr11.ChRate + 1);
        /* получение данных от LTR11 */
        recvd = LTR11_Recv(&hltr11, rbuf, NULL, receivedDataCnt, tout);
        /* Значение меньше нуля соответствуют коду ошибки */
        if (recvd < 0) {
            err = recvd;
            qCritical("Ошибка приема данных. Ошибка %d:%s\n",
                    err, LTR11_GetErrorString(err));
        } else if (recvd!=receivedDataCnt) {
            qCritical("Принято недостаточно данных. Запрашивали %d, приняли %d\n",
                    receivedDataCnt, recvd);
            err = LTR_ERROR_RECV_INSUFFICIENT_DATA;
        } else {
            /* сохранение принятых и обработанных данных в буфере */
            err = LTR11_ProcessData(&hltr11, rbuf, data, &recvd, TRUE, TRUE);
            if (err!=LTR_OK) {
                qCritical("Ошибка обработки данных. Ошибка %d:%s\n",
                        err, LTR11_GetErrorString(err));
            } else {
                /* Принимаем данные */
                receivedBlocks++;
                qDebug("Блок %4d: ", receivedBlocks);
                m_block.insert(m_block.end(), &data[0], &data[receivedDataCnt]);
                emit sendBlock(m_block);
                m_block.clear();
            }


        }
        if (m_abort) {
            closeLtr();
            break;
        }
    }
    delete[] rbuf;
    delete[] data;
}

void Ltr11::closeLtr() {
    if (LTR11_IsOpened(&hltr11) == LTR_OK) {
        /* закрытие канала связи с модулем */
        LTR11_Close(&hltr11);

    }
   // QProcess::startDetached("./ltrd/ltrd.exe", QStringList() << "--stop");
}


void Ltr11::stopProcess() {
    mutex.lock();
    m_abort = true;
    mutex.unlock();
}
