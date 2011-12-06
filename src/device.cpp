#include "device.h"

#include <QDebug>
#include <QString>
#include <QStringList>

Device::Device()
    :QObject(0)
{
}

Device::Device(const Device &other, QObject *parent)
    :QObject(parent)
{
    device_name = other.device_name;
    device_number = other.device_number;
    device_address = other.device_address;
}

Device::Device(const QString &address, QObject *parent)
    :QObject(parent)
{
    device_address = address;
}

Device::Device(const QString &address, const QString &name,
               QObject *parent)
    :QObject(parent)
{
    device_name = name;
    device_address = address;
}

Device::~Device()
{
}
