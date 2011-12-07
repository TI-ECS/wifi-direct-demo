#include "wpa.h"

#include "interfaces.h"
#include "peer.h"

#include <QtDBus>

using namespace fi::w1;
using namespace fi::w1::wpa_supplicant;
using namespace fi::w1::wpa_supplicant::Interface;

static const QString wpa_service = "fi.w1.wpa_supplicant1";

Wpa::Wpa(QObject *parent)
    :QObject(parent)
{
    QString interfacePath;
    wpa_supplicant1 interfaces(wpa_service,
                               "/fi/w1/wpa_supplicant1",
                               QDBusConnection::systemBus());
    QList<QDBusObjectPath> list = interfaces.interfaces();
    if (list.size() == 0) {
        qCritical() << "There is no wpa supplicant interface";
        abort();
    } else {
        interfacePath = list.at(0).path();
    }

    p2pInterface = new P2PDevice(wpa_service, interfacePath,
                              QDBusConnection::systemBus());

    connect(p2pInterface, SIGNAL(DeviceFound(const QDBusObjectPath&, const QVariantMap&)),
            this, SLOT(deviceWasFound(const QDBusObjectPath&, const QVariantMap&)));
    connect(p2pInterface, SIGNAL(GroupStarted(const QVariantMap&)),
            this, SLOT(groupStarted(const QVariantMap&)));

    find();
    getPeers();
}


Wpa::~Wpa()
{
    delete p2pInterface;
}

void Wpa::deviceWasFound(const QDBusObjectPath &path,
                         const QVariantMap &properties)
{
    qDebug() << "path: " << path.path();
}

void Wpa::find()
{
    QDBusPendingCallWatcher *watcher;
    watcher = new QDBusPendingCallWatcher(p2pInterface->Find(QVariantMap()), this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(findResult(QDBusPendingCallWatcher*)));
}

void Wpa::findResult(QDBusPendingCallWatcher *watcher)
{
    watcher->deleteLater();
    QDBusPendingReply<> reply = *watcher;
    if (!reply.isValid()) {
        qDebug() << "Find Fails: " << reply.error().name();
    }
}

void Wpa::getPeers()
{
    QList<QDBusObjectPath> peers = p2pInterface->peers();
    foreach(QDBusObjectPath path, peers) {
        Peer p(wpa_service, path.path(), QDBusConnection::systemBus());
        QVariantMap properties = p.properties();
        QByteArray addr = path.path().split("/").last().toAscii();
        for (int i = 2; i < addr.size(); i+=3)
            addr.insert(i, ':');
        QString deviceName = properties.value("DeviceName").toString();
        Device dev(addr, deviceName);
        emit deviceFound(dev);
    }
}

void Wpa::groupStarted(const QVariantMap &properties)
{
    emit groupHasStarted();
}
