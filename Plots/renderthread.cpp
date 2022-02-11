#include "renderthread.h"
#include <QtMath>
#include <QtCore/QRandomGenerator>

RenderThread::RenderThread(QObject *parent)
    : QObject(parent)
{
    m_abort = false;
}

RenderThread::~RenderThread() {

    m_abort = true;

}

void RenderThread::processGraph() {

    m_abort = false;

}

void RenderThread::receive() {
    double y = 0.0;
    while (true) {
        for (int i = 0; i <= 10; i++) {
            //y = QRandomGenerator::global()->bounded(10) - 5;
            y = 0;
            sine.push_back(y);
        }
        emit sendBlock(sine);
        if (m_abort)
            return;

        sine.clear();
    }
}


void RenderThread::stopProcess()
{

    m_abort = true;

}
