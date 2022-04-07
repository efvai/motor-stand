#ifndef LTR11SETTINGSDIALOG_H
#define LTR11SETTINGSDIALOG_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QTabWidget;
class QLabel;
class QSpinBox;
class QComboBox;
QT_END_NAMESPACE

struct Ltr11Settings {
    int adcFrequency = 10000;
    QString info = "Нет информации";
    int type = 0;
};

namespace Ltr11Tabs {
class GeneralTab : public QWidget
{
    Q_OBJECT

public:
    explicit GeneralTab(QWidget *parent = nullptr);

    QLabel *m_info = nullptr;
    QSpinBox *m_frequency = nullptr;
    QComboBox *m_type = nullptr;
};

class ChannelsTab : public QWidget
{
    Q_OBJECT

public:
    explicit ChannelsTab(QWidget *parent = nullptr);
};

}


class Ltr11SettingsDialog : public QWidget
{
    Q_OBJECT
public:
    explicit Ltr11SettingsDialog(QWidget *parent = nullptr);
    Ltr11Settings settings() const;
    void applySettings();
private:
    QTabWidget *m_tabs = nullptr;
    Ltr11Tabs::ChannelsTab *m_channels = nullptr;
    Ltr11Tabs::GeneralTab *m_general = nullptr;
    Ltr11Settings m_settings;
signals:
    void settingsChanged();
public slots:
    void setInfo(const QString &info);

private slots:
    void on_freqChanged();

};

#endif // LTR11SETTINGSDIALOG_H
