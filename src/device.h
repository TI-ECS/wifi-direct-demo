#ifndef _DEVICE_H_
#define _DEVICE_H_

#include <QMetaType>
#include <QObject>

class Device : public QObject
{
Q_OBJECT

public:
    Device();
    Device(const Device &other, QObject *parent = 0);
    Device(const QString &address, QObject *parent = 0);
    virtual ~Device();

    QString address();
    void setValues(const QString &values);
    QString value();

signals:
    void valueChanged();

private:
    QString device_name;
    QString device_number;
    QString device_address;
};

Q_DECLARE_METATYPE(Device *)

#endif /* _DEVICE_H_ */
