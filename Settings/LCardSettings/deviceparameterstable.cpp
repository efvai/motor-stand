#include "deviceparameterstable.h"

DeviceParametersTableModel::DeviceParametersTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{

}

int DeviceParametersTableModel::rowCount(const QModelIndex &parent) const
{
    return 4;
}

int DeviceParametersTableModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}

QVariant DeviceParametersTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
       return QString("Row%1, Column%2")
                   .arg(index.row() + 1)
                   .arg(index.column() +1);

    return QVariant();
}
