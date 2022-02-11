#include "ltr22.h"
#include <QDebug>

ltr22::ltr22(QObject *parent)
    : QThread(parent)
{
    module = new TLTR22();
    err = 0;
    initModule();
}

ltr22::~ltr22() {
    delete module;
}


void ltr22::initModule() {
    err = LTR22_Close(module);
    err = LTR22_Open(module, module->Channel.saddr, module->Channel.sport,
                     module->Channel.csn, 0);
    if (err==LTR_WARNING_MODULE_IN_USE) {
        qDebug() << "Warning Module LTR22 already opened anywhere \n";
        err = LTR_OK;
    }
    if (err != LTR_OK) {
        qDebug() << "Open ltr22 failed \n";
        return;
    }
    if ((err = LTR22_IsOpened(module)) != LTR_OK) {
        qDebug() << err << "\n";
    }
    else {
        qDebug() << "Module LTR22 Opened Successfull \n";
    }
    err = LTR22_SetFreq(module, false, 1);
    // to do
}

