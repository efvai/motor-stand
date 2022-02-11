#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

#include <QObject>

class RenderThread : public QObject
{
    Q_OBJECT
public:
    RenderThread(QObject *parent = nullptr);
    ~RenderThread();
    void processGraph();

    signals:
        void sendBlock(const std::vector<double> &block);

    public slots:
        void stopProcess();

    public:
        void receive();

    private:
  //      Ltr11 ltr11;
        bool m_abort;
        std::vector<double> sine;


};
//Q_DECLARE_METATYPE(QList<QPointF>);
#endif // RENDERTHREAD_H
