#include "deviceslistmodel.h"

#include "device.h"


DevicesListModel::DevicesListModel(QObject *parent)
    :QAbstractListModel(parent),
     devices()
{
}

DevicesListModel::~DevicesListModel()
{
}

int DevicesListModel::rowCount(const QModelIndex &) const
{
    return devices.count();
}

QVariant DevicesListModel::data(const QModelIndex &index,
                                int role) const
{
    if (!index.isValid())
        return QVariant();

    int devicesSize = devices.size();
    int row = index.row();
    if ((row >= devicesSize) || (row < 0))
        return QVariant();

    if (role == Qt::DisplayRole) {
        Device *it =  devices.at(index.row()).data();
        if (it->name().isEmpty())
            return QVariant(it->address());
        else
            return QVariant(it->name());
    } else if (role == Qt::UserRole) {
        return qVariantFromValue<Device*>(devices.at(index.row()).data());
    } else if (role == Qt::UserRole + 1) {
        return QVariant(devices.at(index.row()).data()->address());
    } else if (role == Qt::UserRole + 2) {
        return QVariant(devices.at(index.row()).data()->name());
    } else
        return QVariant();
}

void DevicesListModel::setDevicesList(const QList<QSharedPointer<Device> > &devices)
{
    this->devices = devices;
    foreach (QSharedPointer<Device> item, devices)
        connect(item.data(), SIGNAL(valueChanged(Device *)), this,
                SLOT(deviceItemChanged(Device *)));
    emit dataChanged(this->createIndex(0, 0),
                     this->createIndex(devices.count(), 0));
}

QVariant DevicesListModel::headerData(int, Qt::Orientation, int) const
{
    return QVariant();
}

void DevicesListModel::deviceItemChanged(Device *item)
{
    int i = 0;
    foreach (QSharedPointer<Device> it, devices) {
        if (it.data() == item) {
            emit dataChanged(this->createIndex(i, 0),
                             this->createIndex(i, 0));
            return;
        }
        i++;
    }
}
