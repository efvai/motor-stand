#ifndef MOTORCONTROLWIDGET_H
#define MOTORCONTROLWIDGET_H

#include <QWidget>

namespace Ui {
class MotorControlWidget;
}

class Ltr34;

class MotorControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MotorControlWidget(QWidget *parent = nullptr);
    ~MotorControlWidget();

public slots:
    void enableDacControls();
    void disableDacControls();

private slots:
    void dacSliderResponse();
    void dacValueChanged();
    void zeroBtnClicked();
    void dacStatusChanged();
    void processDacResponse();
    void processDacError(const QString &err);

private:
    double scaleDacSliderValue(double scale);

signals:
    void dacdac();

private:
    Ltr34 *m_dac = nullptr;

private:
    Ui::MotorControlWidget *ui;
};

#endif // MOTORCONTROLWIDGET_H
