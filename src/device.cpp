#include "device.h"

Device::Device(const QString &address)
{
    this->address = address;
}

Device::~Device()
{
}

void Device::setValues(const QString &values)
{
}

const QString Device::value()
{
    return address;
}
