#ifndef GLOBALPLOTSETTINGSDIALOG_H
#define GLOBALPLOTSETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class GlobalPlotSettingsDialog;
}

struct GlobalPlotSettings {
    int plotsCount = 3;

};

class GlobalPlotSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GlobalPlotSettingsDialog(QWidget *parent = nullptr);
    ~GlobalPlotSettingsDialog();

    GlobalPlotSettings settings() const;

private:
    Ui::GlobalPlotSettingsDialog *ui;
    GlobalPlotSettings m_settings;

signals:
    void settingsChanged();
};

#endif // GLOBALPLOTSETTINGSDIALOG_H
