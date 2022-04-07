#include "ltr22settingsdialog.h"
#include "ltr22api.h"
#include <QTabWidget>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QLayout>

Ltr22SettingsDialog::Ltr22SettingsDialog(QWidget *parent)
    : QWidget(parent)
{
    m_tabs = new QTabWidget;
    m_channels = new Ltr22Tabs::ChannelsTab;
    m_general = new Ltr22Tabs::GeneralTab;
    m_tabs->addTab(m_general, tr("Главное"));
    m_tabs->addTab(m_channels, tr("Каналы"));
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_tabs);
    setLayout(mainLayout);

    m_general->m_acdc->setChecked(m_settings.acdc);
    m_general->m_zeroMeas->setChecked(m_settings.zeroMeas);
    m_general->m_frequency->setValue(m_settings.adcFrequency);
    m_general->m_info->setText(m_settings.info);

    m_general->m_frequency->setRange(0, 100000);
    connect(m_general->m_frequency, &QSpinBox::editingFinished, this, &Ltr22SettingsDialog::on_freqChanged);

}

Ltr22Settings Ltr22SettingsDialog::settings() const
{
    return m_settings;
}

void Ltr22SettingsDialog::setInfo(const QString &info)
{
    m_general->m_info->setText(info);

}

void Ltr22SettingsDialog::on_freqChanged()
{
    double resultFreq = 0.0;
    LTR22_FindAdcFreqParams(m_general->m_frequency->value(), nullptr, nullptr, nullptr, &resultFreq);
    m_general->m_frequency->setValue(resultFreq);
}

void Ltr22SettingsDialog::applySettings()
{
    m_settings.adcFrequency = m_general->m_frequency->value();
    m_settings.acdc = m_general->m_acdc->isChecked();
    m_settings.zeroMeas = m_general->m_zeroMeas->isChecked();
}

Ltr22Tabs::GeneralTab::GeneralTab(QWidget *parent)
    : QWidget(parent)
{
    QLabel *infoLabel = new QLabel(tr("Информация:"));
    QLabel *frequencyLabel = new QLabel(tr("Частота АЦП:"));
    QLabel *acdcLabel = new QLabel(tr("Включить режим компенсации постоянной составляющей"));
    m_acdc = new QCheckBox;
    QLabel *zeroMeasLabel = new QLabel(tr("Включить режим измерения собственного нуля"));
    m_zeroMeas = new QCheckBox;
    m_info = new QLabel;
    m_frequency = new QSpinBox;
    m_frequency->setMaximum(400000);
    m_frequency->setMinimum(5);
    m_info->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(infoLabel);
    mainLayout->addWidget(m_info);
    mainLayout->addWidget(frequencyLabel);
    mainLayout->addWidget(m_frequency);
    mainLayout->addWidget(acdcLabel);
    mainLayout->addWidget(m_acdc);
    mainLayout->addWidget(zeroMeasLabel);
    mainLayout->addWidget(m_zeroMeas);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

Ltr22Tabs::ChannelsTab::ChannelsTab(QWidget *parent)
    : QWidget(parent)
{

}
