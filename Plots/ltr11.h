#ifndef LTR11_H
#define LTR11_H

//#include "ltr/include/ltr11api.h"
#include "../Libs/ltr/include/ltr11api.h"
#include <QObject>
#include <QMutex>
#include <QThread>
#include <vector>
#include <QWaitCondition>

typedef struct {
    int slot; // Номер слота
    const char *serial; // Серийный номер крейта
    DWORD addr; // ip-адрес сервера
} t_open_param;

class Ltr11Settings;

class Ltr11 : public QThread
{
     Q_OBJECT
public:
    Ltr11(QObject *parent = nullptr); // Если slot = -1 => По умолчанию 1 слот
    ~Ltr11();
protected:
    void run();
signals:
    void sendStatus();
    void sendInfo(QString &info);
public slots:
    void stopProcess();


private:
    void getParams(int slot);
    void init();
    void configure();
    void setStatus(const QString &status);
public:
    void transaction();

private:
    std::vector<double> m_block;

    bool m_abort = false;
    bool m_pause = true;
    QMutex m_mutex;
    QWaitCondition m_cond;

    TLTR11 hltr11;
    int m_ltr11_error = 0;
    t_open_param param;
    const int RECV_BLOCK_CH_SIZE = 1024;
    const int RECV_TOUT = 4000;
    Ltr11Settings *settings = nullptr;
    QString moduleInfo = "Нет информации!";
    int slot = 2;

    float time = 0.0f;
public:
    void closeLtr();
    void setSlot(int slot);
    bool pause() const;
    void setPause(bool newPause);
    void setSettings(Ltr11Settings newSettings);
    const QString &getModuleInfo() const;
};
Q_DECLARE_METATYPE(std::vector<double>);
#endif // LTR11_H
