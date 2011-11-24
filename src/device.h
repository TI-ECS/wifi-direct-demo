#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <QObject>

class Device
{

public:
    Device(const QString &address);
    virtual ~Device();

    void setValues(const QString &values);
    const QString value();

private:
    QString device_name;
    QString device_number;
    QString address;
};
#endif /* _DEVICE_H_ */
