#include "wpa.h"

#include "interfaces.h"
#include "peer.h"

#include <QtDBus>
#include <signal.h>
#include <sys/types.h>

using namespace fi::w1;
using namespace fi::w1::wpa_supplicant;
using namespace fi::w1::wpa_supplicant::Interface;

static const QString wpa_process_name = "wpa_supplicant";
static const QString wpa_service = "fi.w1.wpa_supplicant1";

Q_PID proc_find(const QString &name)
{
    bool ok;
    QDir dir;

    dir = QDir("/proc");
    if (!dir.exists()) {
        qCritical() << "can't open /proc";
        return -1;
    }

    foreach(QString fileName, dir.entryList()) {
        long lpid = fileName.toLong(&ok, 10);
        if (!ok)
            continue;

        QFile file(QString("/proc/%1/cmdline").arg(lpid));
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray cmd = file.readLine();
            if (cmd.contains(name.toAscii())) {
                file.close();
                return (Q_PID)lpid;
            }
            file.close();
        }
    }

    return -1;
}

Wpa::Wpa(QObject *parent)
    :QObject(parent)
{
    group = NULL;
    wpaPid = proc_find(wpa_process_name);

    if (wpaPid != -1) {
	enabled(true);
	setupDBus();
    } else {
	enabled(false);
    }
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

void Wpa::disconnect()
{
    QDBusPendingCallWatcher *watcher;
    watcher = new QDBusPendingCallWatcher(p2pInterface->Disconnect(), this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(disconnectResult(QDBusPendingCallWatcher*)));
}

void Wpa::disconnectResult(QDBusPendingCallWatcher *watcher)
{
    watcher->deleteLater();
    QDBusPendingReply<> reply = *watcher;
    if (!reply.isValid()) {
        qDebug() << "Disconnect Fails: " << reply.error().name();
    } else {
	emit disconnected();
    }
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

void Wpa::groupHasStarted(const QVariantMap &properties)
{
    emit groupStarted();
}

void Wpa::groupHasFinished(const QString &ifname, const QString &role)
{
    emit groupFinished();
}

void Wpa::groupStartResult(QDBusPendingCallWatcher *watcher)
{
    watcher->deleteLater();
    QDBusPendingReply<> reply = *watcher;
    if (!reply.isValid()) {
        qDebug() << "Group Start Fails: " << reply.error().name();
	emit groupStartFails();
    } else {
    }
}

bool Wpa::isEnabled()
{
    return (wpaPid == -1) ? false : true;
}

void Wpa::peerJoined(const QDBusObjectPath &peer)
{
    qDebug() << "peer connected: " << peer.path();
}

// void Wpa::provisionDiscoveryPBCRequest(const QDBusObjectPath &peer_object)
// {
//     QVariantMap args;
//     args["Role"] = wps_role;
//     args["Type"] = "pbc";

//     QDBusPendingCallWatcher *watcher;
//     watcher = new QDBusPendingCallWatcher(
//         p2pInterface->ProvisionDiscoveryRequest(peer_object, "pbc"), this);
// }

void Wpa::setEnabled(bool enable)
{
    if (enable && (wpaPid != -1)) {
        if (QProcess::startDetached(wpa_process_name,
                                    QStringList() << "d" << "-Dnl80211"
                                    << "-c/etc/wpa_supplicant.conf" << "-iwlan0"
                                    << "-B", QDir::rootPath(), &wpaPid)
	    && (wpaPid > 0)) {
            wpaPid += 1;        // It's really weird, but startDetached is
                                // it's always returning the pid - 1.
            sleep(4);
	    setupDBus();
	    emit enabled(true);
	}
    } else {
	if (kill(wpaPid, SIGKILL) != -1) {
	    p2pInterface->disconnect();
	    delete p2pInterface;
	    p2pInterface = NULL;
            wpaPid = -1;
            emit enabled(false);
	}
    }
}

void Wpa::setupDBus()
{
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

    // InterfaceDevice interf(wpa_service,
    //                        interfacePath,
    //                        QDBusConnection::systemBus());
    // interf.setApScan(0);

    p2pInterface = new P2PDevice(wpa_service, interfacePath,
                              QDBusConnection::systemBus());
    connect(p2pInterface, SIGNAL(DeviceFound(const QDBusObjectPath&)),
            this, SLOT(deviceWasFound(const QDBusObjectPath&)));
    connect(p2pInterface, SIGNAL(GroupStarted(const QVariantMap&)),
            this, SLOT(groupHasStarted(const QVariantMap&)));
    connect(p2pInterface, SIGNAL(GroupFinished(const QString&, const QString&)),
            this, SLOT(groupHasFinished(const QString&, const QString&)));
    connect(p2pInterface, SIGNAL(P2PStateChanged(const QStringMap&)),
            this, SLOT(stateChanged(const QStringMap&)));
    connect(p2pInterface, SIGNAL(GONegotiationSuccess()), this,
            SIGNAL(connected()));
    connect(p2pInterface, SIGNAL(ProvisionDiscoveryPBCRequest(const QDBusObjectPath&)),
            this, SLOT(provisionDiscoveryPBCRequest(const QDBusObjectPath&)));

    // wps = new WPS(wpa_service, interfacePath,
    //               QDBusConnection::systemBus());
    // wps->setProcessCredentials(true);
    // connect(wps, SIGNAL(Event(const QString&, const QVariantMap&)), this,
    //         SLOT(wpsEvent(const QString&, const QVariantMap&)));

    find();
    getPeers();
}

void Wpa::stateChanged(const QStringMap &states)
{
}

void Wpa::startGroup()
{
    QDBusPendingCallWatcher *watcher;
    QVariantMap args;
    args["persistent"] = true;
    args["frequency"] = 2;      // Using 2.4 Ghz
    watcher = new QDBusPendingCallWatcher(p2pInterface->GroupAdd(args), this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(groupHasStarted(QDBusPendingCallWatcher*)));
}

void Wpa::stopGroup()
{
}
