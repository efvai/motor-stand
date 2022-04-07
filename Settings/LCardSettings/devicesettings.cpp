#include "devicesettings.h"
#include "ui_devicesettings.h"

#include <QListView>
#include <QStringListModel>
#include <QTableView>
#include <QHeaderView>
#include <QLayout>
#include <QDebug>
#include <QPushButton>

#include "deviceparameterstable.h"

deviceSettings::deviceSettings(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::deviceSettings)
{
    ui->setupUi(this);
    QPushButton *applyBtn = new QPushButton(tr("Сохранить и выйти"));
    QPushButton *cancelBtn = new QPushButton(tr("Отменить"));

    m_ltr11 = new Ltr11SettingsDialog;
    m_ltr22 = new Ltr22SettingsDialog;
    m_ltr22->hide();
    QTableView *tableView = new QTableView();
    DeviceParametersTableModel *tableModel = new DeviceParametersTableModel();
    tableView->setModel(tableModel);
    tableView->horizontalHeader()->hide();
    QListView *listView = new QListView();
    listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QStringListModel *model = new QStringListModel();
    QStringList list;
    list << "LTR 11" << "LTR 22";
    model->setStringList(list);
    listView->setModel(model);
    QGridLayout *mainLayout = new QGridLayout(this);
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(applyBtn);
    buttonsLayout->addWidget(cancelBtn);
    QGridLayout *leftGrid = new QGridLayout;
    leftGrid->addWidget(listView, 0, 0, 3, 0);
    leftGrid->addLayout(buttonsLayout, 4, 0);
    mainLayout->addLayout(leftGrid, 0, 0);
    mainLayout->addWidget(m_ltr11, 0, 1);
    mainLayout->addWidget(m_ltr22, 0, 1);
    mainLayout->setColumnMinimumWidth(1, 600);
    setLayout(mainLayout);


    listView->setCurrentIndex(model->index(0,0));
    connect(listView, &QListView::clicked, this, &deviceSettings::on_selectionChange);

    connect(applyBtn, &QPushButton::clicked, this, &deviceSettings::on_saveBtnClicked);
    connect(cancelBtn, &QPushButton::clicked, this, &QWidget::hide);
}

deviceSettings::~deviceSettings()
{
    delete ui;
}

void deviceSettings::on_selectionChange(const QModelIndex &index)
{
    if (index.row() == 1) {
        m_ltr11->hide();
        m_ltr22->show();
    } else if (index.row() == 0) {
        m_ltr22->hide();
        m_ltr11->show();
    }

}

void deviceSettings::on_saveBtnClicked()
{
    m_ltr11->applySettings();
    m_ltr22->applySettings();
    emit settingsChanged();
    hide();
}

void deviceSettings::setLtr11Info(const QString &info)
{
    m_ltr11->setInfo(info);
}

void deviceSettings::setLtr22Info(const QString &info)
{
    m_ltr22->setInfo(info);
}

Ltr11Settings deviceSettings::ltr11Settings() const
{
    return m_ltr11->settings();
}

Ltr22Settings deviceSettings::ltr22Settings() const
{
    return m_ltr22->settings();
}

