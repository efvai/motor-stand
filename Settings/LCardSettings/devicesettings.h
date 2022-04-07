#ifndef DEVICESETTINGS_H
#define DEVICESETTINGS_H

#include <QWidget>
#include "ltr11settingsdialog.h"
#include "ltr22settingsdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class deviceSettings; }
QT_END_NAMESPACE


class deviceSettings : public QWidget
{
    Q_OBJECT

public:
    deviceSettings(QWidget *parent = nullptr);
    ~deviceSettings();
    void setLtr11Info(const QString &info);
    void setLtr22Info(const QString &info);
    Ltr11Settings ltr11Settings() const;
    Ltr22Settings ltr22Settings() const;

private slots:
    void on_selectionChange(const QModelIndex &index);
    void on_saveBtnClicked();



signals:
    void settingsChanged();

private:
    Ui::deviceSettings *ui;
    Ltr11SettingsDialog *m_ltr11;
    Ltr22SettingsDialog *m_ltr22;
};
#endif // DEVICESETTINGS_H
