#include "deviceslistmodel.h"

#include "device.h"


DevicesListModel::DevicesListModel(QObject *parent)
    :QAbstractListModel(parent),
     devices()
{
}

DevicesListModel::~DevicesListModel()
{
    foreach (Device *d, devices)
        delete d;

    devices.clear();
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
        Device *it =  devices.at(index.row());
        if (it->name().isEmpty())
            return QVariant(it->address());
        else
            return QVariant(it->name());
    } else if (role == Qt::UserRole) {
        return qVariantFromValue<Device*>(devices.at(index.row()));
    } else if (role == Qt::UserRole + 1) {
        return QVariant(devices.at(index.row())->address());
    } else if (role == Qt::UserRole + 2) {
        return QVariant(devices.at(index.row())->name());
    } else
        return QVariant();
}

void DevicesListModel::addDevice(const Device &device)
{
    int pos = devices.count();
    Device *d = new Device(device);

    devices.append(d);
    connect(d, SIGNAL(valueChanged(Device *)), this,
            SLOT(deviceItemChanged(Device *)));

    emit dataChanged(this->createIndex(pos, 0),
                     this->createIndex(pos + 1, 0));
}

QVariant DevicesListModel::headerData(int, Qt::Orientation, int) const
{
    return QVariant();
}

void DevicesListModel::deviceItemChanged(Device *item)
{
    int i = 0;
    foreach (Device* it, devices) {
        if (it == item) {
            emit dataChanged(this->createIndex(i, 0),
                             this->createIndex(i, 0));
            return;
        }
        i++;
    }
}
