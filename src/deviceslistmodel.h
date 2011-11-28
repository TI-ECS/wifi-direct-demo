#ifndef _DEVICESLISTMODEL_H_
#define _DEVICESLISTMODEL_H_

#include <QAbstractListModel>
#include <QSharedPointer>

class Device;

class DevicesListModel : public QAbstractListModel
{
Q_OBJECT

public:
    DevicesListModel(QObject *parent = 0);
    virtual ~DevicesListModel();

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
				int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex &index = QModelIndex()) const;

public slots:
    void setDevicesList(const QList<QSharedPointer<Device> > &devices);

private slots:
    void deviceItemChanged(Device *item);

private:
    QList<QSharedPointer<Device> > devices;
};

#endif /* _DEVICESLISTMODEL_H_ */
