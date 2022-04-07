#include "motorcontrolwidget.h"
#include "ui_motorcontrolwidget.h"

#include "ltr34.h"

#include <QDebug>
#include <QSlider>
#include <QPushButton>

MotorControlWidget::MotorControlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MotorControlWidget)
{
    m_dac = new Ltr34;

    ui->setupUi(this);



    connect(ui->dacSlider, &QSlider::valueChanged, this, &MotorControlWidget::dacSliderResponse);
    connect(ui->dacSlider, &QSlider::sliderReleased, this, &MotorControlWidget::dacValueChanged);
    connect(ui->dacZeroBtn, &QPushButton::clicked, this, &MotorControlWidget::zeroBtnClicked);
    connect(m_dac, &Ltr34::error, this, &MotorControlWidget::processDacError);
    connect(m_dac, &Ltr34::response, this, &MotorControlWidget::processDacResponse);
}

MotorControlWidget::~MotorControlWidget()
{
    delete m_dac;
    delete ui;
}

void MotorControlWidget::enableDacControls()
{
    ui->dacSlider->setEnabled(true);
    ui->dacZeroBtn->setEnabled(true);
}

void MotorControlWidget::disableDacControls()
{
    ui->dacSlider->setEnabled(false);
    ui->dacZeroBtn->setEnabled(false);
}

void MotorControlWidget::dacSliderResponse()
{
    ui->dacLcd->display(scaleDacSliderValue(0.1));
}

void MotorControlWidget::dacValueChanged()
{
    disableDacControls();
    m_dac->transaction(scaleDacSliderValue(0.1));
}

void MotorControlWidget::zeroBtnClicked()
{
    ui->dacSlider->setValue(0);
    dacValueChanged();
}

void MotorControlWidget::dacStatusChanged()
{
    emit dacdac();
}

void MotorControlWidget::processDacResponse()
{
    enableDacControls();
    ui->dacLabel->setText(QString("Напряжение: %1 В").arg(scaleDacSliderValue(0.1)));
}

void MotorControlWidget::processDacError(const QString &err)
{
    enableDacControls();
    ui->dacLabel->setText(err);
}

double MotorControlWidget::scaleDacSliderValue(double scale)
{
    return scale * ui->dacSlider->value();
}
