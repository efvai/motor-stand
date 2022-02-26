#include "globalplotsettingsdialog.h"
#include "ui_globalplotsettingsdialog.h"

GlobalPlotSettingsDialog::GlobalPlotSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GlobalPlotSettingsDialog)
{
    ui->setupUi(this);
    ui->plotCountSpinner->setValue(m_settings.plotsCount);
    connect(ui->applyBtn, &QPushButton::clicked, [this]() {

        m_settings.plotsCount = ui->plotCountSpinner->value();
        emit settingsChanged();
        hide();
    });
}

GlobalPlotSettingsDialog::~GlobalPlotSettingsDialog()
{
    delete ui;
}

GlobalPlotSettings GlobalPlotSettingsDialog::settings() const
{
    return m_settings;
}
