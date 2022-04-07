#ifndef LTR22_H
#define LTR22_H

#include <QObject>
#include <QThread>
#include <QObject>
#include <QMutex>
#include "../Libs/ltr/include/ltr22api.h"
#include <QWaitCondition>

/* Fclkin = 20 * 10^6 / fdiv_main
   Fch = Fclkin / (128 * fdiv_extra)
   fdiv_main = 1, 2, ... 14, 15
   fdiv_extra = false (2), true (3)

   table of freq: : 3472; 3720; 4006; 4340;
                    4735; 5208; 5580; 5787;
                    6010; 6510; 7102; 7440;
                    7812,5; 8681; 9766; 10417;
                    11161; 13021; 13021; 15625;
                    17361; 19531; 26042; 26042;
                    39062,5; 52083; 78125
*/

class Ltr22Settings;

struct freq {
    int fdiv_main = 0;
    int fdiv_extra = 0;
    int frequency = 0;
};

struct open_param{
    int slot; // Номер слота
    const char *serial; // Серийный номер крейта
    DWORD addr; // ip-адрес сервера
    WORD sport;
};

class Ltr22 : public QThread
{
    Q_OBJECT
public:
    Ltr22(QObject *parent = nullptr);
    ~Ltr22();

protected:
    void run();

private:
    TLTR22 *m_module;
    int m_ltr22_error;
    freq m_frequency; // why?
    open_param m_param;
    int slot = 3; // why?
    int m_activeChannels = 1;

    const int RECV_BLOCK_CH_SIZE = 500;
    const int RECV_TOUT = 4000;
    float time = 0.0f;

    QString moduleInfo = "Нет информации!";
    Ltr22Settings *settings = nullptr;

private:
    void initModule();
    void closeModule();
    void configureModule();
    void getParams(int slot);

private:
    bool m_abort = false;
    bool m_pause = true;
    QMutex m_mutex;
    QWaitCondition m_cond;

public:
    bool pause() const;
    void setPause(bool newPause);
    void transaction();
    void setSettings(Ltr22Settings newSettings);
    const QString &getModuleInfo() const;

signals:
    void sendStatus();
    void sendInfo(QString &info);
private:
    void setStatus(const QString &status, int ch, bool all);

};

#endif // LTR22_H
