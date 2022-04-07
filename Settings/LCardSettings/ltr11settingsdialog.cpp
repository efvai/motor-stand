#include "ltr11settingsdialog.h"

#include <QTabWidget>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QLayout>
#include "ltr11api.h"

Ltr11SettingsDialog::Ltr11SettingsDialog(QWidget *parent)
    : QWidget(parent)
{
    m_tabs = new QTabWidget;
    m_channels = new Ltr11Tabs::ChannelsTab;
    m_general = new Ltr11Tabs::GeneralTab;
    m_tabs->addTab(m_general, tr("Главное"));
    m_tabs->addTab(m_channels, tr("Каналы"));
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_tabs);
    setLayout(mainLayout);

    m_general->m_type->setCurrentIndex(m_settings.type);
    m_general->m_frequency->setValue(m_settings.adcFrequency);
    m_general->m_info->setText(m_settings.info);
    m_general->m_frequency->setRange(1, 400000);
    connect(m_general->m_frequency, &QSpinBox::editingFinished, this, &Ltr11SettingsDialog::on_freqChanged);
}

Ltr11Settings Ltr11SettingsDialog::settings() const
{
    return m_settings;
}

void Ltr11SettingsDialog::setInfo(const QString &info)
{
    m_general->m_info->setText(info);

}

void Ltr11SettingsDialog::on_freqChanged()
{
    double resultFreq = 0.0;
    LTR11_FindAdcFreqParams(m_general->m_frequency->value(), nullptr, nullptr, &resultFreq);
    m_general->m_frequency->setValue(resultFreq);
}

void Ltr11SettingsDialog::applySettings()
{
    m_settings.adcFrequency = m_general->m_frequency->value();
    m_settings.type = m_general->m_type->currentIndex();
}

Ltr11Tabs::GeneralTab::GeneralTab(QWidget *parent)
    : QWidget(parent)
{
    QLabel *infoLabel = new QLabel(tr("Информация:"));
    QLabel *frequencyLabel = new QLabel(tr("Частота АЦП:"));
    QLabel *typeLabel = new QLabel(tr("Тип подключения:"));
    m_type = new QComboBox;
    m_type->addItem(tr("Дифф. подключение (16 каналов)"));
    m_type->addItem(tr("Подключение с общей землей (32 канала)"));
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
    mainLayout->addWidget(typeLabel);
    mainLayout->addWidget(m_type);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

Ltr11Tabs::ChannelsTab::ChannelsTab(QWidget *parent)
    : QWidget(parent)
{

}
