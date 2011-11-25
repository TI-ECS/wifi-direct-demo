#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <QObject>

class Device
{

public:
    Device();
    Device(const QString &address);
    virtual ~Device();

    QString address();
    void setValues(const QString &values);
    QString value();

private:
    QString device_name;
    QString device_number;
    QString device_address;
};
#endif /* _DEVICE_H_ */
