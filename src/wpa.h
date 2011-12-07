#ifndef _WPA_H_
#define _WPA_H_

#include "device.h"
#include "p2pdevice.h"

#include <QObject>

class Wpa : public QObject
{
Q_OBJECT

public:
    Wpa(QObject *parent = 0);
    virtual ~Wpa();

public slots:
    void startGroup();
    void stopGroup();
    void disconnect();

private slots:
    void deviceWasFound(const QDBusObjectPath&, const QVariantMap&);
    void disconnectResult(QDBusPendingCallWatcher *watcher);
    void findResult(QDBusPendingCallWatcher *watcher);
    void groupHasStarted(const QVariantMap &properties);
    void groupHasFinished(const QString &ifname, const QString &role);
    void stateChanged(const QStringMap &states);

signals:
    void deviceFound(const Device &device);
    void disconnected();
    void status(const QString &status);
    void groupStarted();
    void groupFinished();

private:
    fi::w1::wpa_supplicant::Interface::P2PDevice *p2pInterface;

    void find();
    void getPeers();
};

#endif /* _WPA_H_ */
