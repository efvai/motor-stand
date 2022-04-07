#ifndef LTR22SETTINGSDIALOG_H
#define LTR22SETTINGSDIALOG_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QTabWidget;
class QLabel;
class QSpinBox;
class QCheckBox;
QT_END_NAMESPACE

struct Ltr22Settings {
    int adcFrequency = 3472;
    QString info = "Нет информации";
    bool acdc = false;
    bool zeroMeas = false;
};

namespace Ltr22Tabs {
class GeneralTab : public QWidget
{
    Q_OBJECT

public:
    explicit GeneralTab(QWidget *parent = nullptr);

    QLabel *m_info = nullptr;
    QSpinBox *m_frequency = nullptr;
    QCheckBox *m_acdc = nullptr;
    QCheckBox *m_zeroMeas = nullptr;
};

class ChannelsTab : public QWidget
{
    Q_OBJECT

public:
    explicit ChannelsTab(QWidget *parent = nullptr);
};
}

class Ltr22SettingsDialog : public QWidget
{
    Q_OBJECT
public:
    explicit Ltr22SettingsDialog(QWidget *parent = nullptr);
    Ltr22Settings settings() const;
    void applySettings();
private:
    QTabWidget *m_tabs = nullptr;
    Ltr22Tabs::ChannelsTab *m_channels = nullptr;
    Ltr22Tabs::GeneralTab *m_general = nullptr;
    Ltr22Settings m_settings;
signals:
    void settingsChanged();
public slots:
    void setInfo(const QString &info);

private slots:
    void on_freqChanged();

};

#endif // LTR22SETTINGSDIALOG_H
