#ifndef PLOTSETTINGSDIALOG_H
#define PLOTSETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class PlotSettingsDialog;
}

struct PlotSettings {
    float yMin = -0.01;
    float yMax = 0.01;
    float xMax = 10.0;
    bool legendEnabled = true;
    int legendAlignment = 2;
};

class PlotSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlotSettingsDialog(QWidget *parent = nullptr);
    ~PlotSettingsDialog();

    PlotSettings settings() const;

private:
    Ui::PlotSettingsDialog *ui;
    PlotSettings m_settings;

signals:
    void settingsChanged();
};

#endif // PLOTSETTINGSDIALOG_H
