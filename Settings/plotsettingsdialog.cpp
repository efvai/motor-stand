#include "plotsettingsdialog.h"
#include "ui_plotsettingsdialog.h"
#include <QMessageBox>

PlotSettingsDialog::PlotSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlotSettingsDialog)
{
    ui->setupUi(this);
    ui->yMinSpinner->setValue(m_settings.yMin);
    ui->yMaxSpinner->setValue(m_settings.yMax);
    ui->xMaxSpinner->setValue(m_settings.xMax);
    if (m_settings.legendEnabled)
        ui->legendCheck->setCheckState(Qt::Checked);
    else
        ui->legendCheck->setCheckState(Qt::Unchecked);
    connect(ui->ApplyButton, &QPushButton::clicked, [this]() {
        float yMin = ui->yMinSpinner->value();
        float yMax = ui->yMaxSpinner->value();
        if (yMin >= yMax) {
            QMessageBox::warning(this, tr("Предупреждение"),
                                           tr("Неправильно введен диапазон оси Y."));
            ui->yMinSpinner->setValue(m_settings.yMin);
            ui->yMaxSpinner->setValue(m_settings.yMax);
            return;
        }
        m_settings.yMin = yMin;
        m_settings.yMax = yMax;
        m_settings.xMax = ui->xMaxSpinner->value();
        if (ui->legendCheck->checkState() == Qt::Checked)
            m_settings.legendEnabled = true;
        else
            m_settings.legendEnabled = false;
        emit settingsChanged();
        hide();
    });
}

PlotSettingsDialog::~PlotSettingsDialog()
{
    delete ui;
}

PlotSettings PlotSettingsDialog::settings() const
{
    return m_settings;
}
