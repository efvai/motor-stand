#ifndef DEVICEPARAMETERSTABLE_H
#define DEVICEPARAMETERSTABLE_H

#include <QAbstractTableModel>

class DeviceParametersTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    DeviceParametersTableModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

};

#endif // DEVICEPARAMETERSTABLE_H
