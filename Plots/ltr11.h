#ifndef LTR11_H
#define LTR11_H

//#include "ltr/include/ltr11api.h"
#include "../Libs/ltr/include/ltr11api.h"
#include <QObject>
#include <QMutex>
#include <QThread>
#include <vector>


typedef struct {
    int slot; // Номер слота
    const char *serial; // Серийный номер крейта
    DWORD addr; // ip-адрес сервера
} t_open_param;

class Ltr11 : public QThread
{
     Q_OBJECT
public:
    Ltr11(QObject *parent = nullptr); // Если slot = -1 => По умолчанию 1 слот
    ~Ltr11();
protected:
    void run();
signals:
    void sendBlock(const std::vector<double> &block);
public slots:
    void stopProcess();
private:
    void getParams(int slot);
    void init();
    void configure();
public:
    void processReceive();

private:
    std::vector<double> m_block;
    bool m_abort;
    QMutex mutex;
    TLTR11 hltr11;
    int err;
    t_open_param param;
    const int RECV_BLOCK_CH_SIZE = 4096 * 2;
    const int RECV_TOUT = 4000;
    const int ACD_FREQ = 10000;
    int slot = 2;
public:
    void receive();
    void closeLtr();
    void setSlot(int slot);
};
Q_DECLARE_METATYPE(std::vector<double>);
#endif // LTR11_H
