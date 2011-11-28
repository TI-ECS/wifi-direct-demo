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

Device::~Device()
{
}

void Device::setValues(const QString &values)
{
    static int count;
    QStringList fields = values.split("\n");

    count = 0;
    foreach (QString f, fields) {
        if (f.startsWith("device_name")) {
            device_name = f.mid(12); //12 == device_name=
            count++;
        } else if (f.startsWith("device_number")) {
            device_number = f.mid(14); //14 == device_number=
            count++;
        } else if (f.startsWith("address")) {
            device_address = f.mid(8); //8 == address=
            count++;
        }

        if (count == 3)
            break;
    }

    emit valueChanged(this);
}
