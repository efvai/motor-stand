#ifndef LTR22_H
#define LTR22_H

#include <QThread>
#include <QObject>
#include "../Libs/ltr/include/ltr22api.h"

class ltr22 : public QThread
{
public:
    ltr22(QObject *parent = nullptr);
    ~ltr22();
private:
    TLTR22 *module;
    int err;

private:
    void initModule();
    void closeModule();
};

#endif // LTR22_H
