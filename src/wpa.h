#ifndef _WPA_H_
#define _WPA_H_

#include "p2pdevice.h"

#include <QObject>

class Wpa : public QObject
{
Q_OBJECT

public:
    Wpa(QObject *parent = 0);
    virtual ~Wpa();

private slots:
    void deviceFound(const QDBusObjectPath&, const QVariantMap&);
    void findResult(QDBusPendingCallWatcher *watcher);
    void groupStarted(const QVariantMap&);

signals:
    void status(const QString &status);
    void groupHasStarted();

private:
    fi::w1::wpa_supplicant::Interface::P2PDevice *p2pInterface;

    void find();
    void getPeers();
};

#endif /* _WPA_H_ */
