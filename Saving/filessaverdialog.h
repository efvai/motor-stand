#ifndef FILESSAVERDIALOG_H
#define FILESSAVERDIALOG_H

#include <QWidget>


QT_BEGIN_NAMESPACE
class QCheckBox;
class QPushButton;
QT_END_NAMESPACE

class FilesSaverDialog : public QWidget
{
    Q_OBJECT
public:
    explicit FilesSaverDialog(QWidget *parent = nullptr);
    void setLtr11Freq(int newLtr11Freq);
    void setLtr22Freq(int newLtr22Freq);

private:
    QList<QCheckBox*> devices;
    QCheckBox *filesInfoCheckBox = nullptr;
    QPushButton *saveBtn = nullptr;
    int m_ltr11Freq = 0;
    int m_ltr22Freq = 0;

signals:

public slots:
    void openSaver();

private slots:
    void on_save();

};

#endif // FILESSAVERDIALOG_H
