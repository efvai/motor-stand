#ifndef LTR34_H
#define LTR34_H

#include "../Libs/ltr/include/ltr34api.h"
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QObject>

/* Voltage source input = (0 - 10) V; Co Output = (0 - 24) V;  1 V = 2.4 V
 * Powder brake input = (0 - 24) V;
 */

struct ltr34_open_param{
    int slot = LTR_CC_CHNUM_MODULE5; // Номер слота
    const char *serial = ""; // Серийный номер крейта
    DWORD addr = SADDR_DEFAULT; // ip-адрес сервера
    WORD sport = SPORT_DEFAULT;
};

class Ltr34 : public QThread
{
    Q_OBJECT
public:
    Ltr34(QObject *parent = nullptr);
    ~Ltr34();

protected:
    void run();

public:
    void transaction(float request);

private:
    bool m_abort = false;
    QMutex m_mutex;
    QWaitCondition m_cond;

private:
    TLTR34 *m_module = nullptr;
    int m_ltr34_error = 0;
    ltr34_open_param m_param;

private:
    void getModuleInfo();
    void configureModule();

private:
    double req;

signals:
    void sendStatus();
    void sendInfo(QString &info);
    void response();
    void error(const QString &error);

private:
    void setStatus(const QString &status);

private:
    QString moduleInfo = "Нет информации!";

};

#endif // LTR34_H
