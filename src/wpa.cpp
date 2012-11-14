#include "wpa.h"

#include "interface.h"
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
static const QString wps_role = "enrollee";

static Q_PID proc_find(const QString &name)
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
            QByteArray cmd = file.readAll();
            cmd = cmd.replace('\0', ' ');
            cmd.append('\0');
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
    p2pInterface = NULL;
    device = NULL;

    wpaPid = proc_find(wpa_process_name);
    if (wpaPid != -1) {
        setupDBus();
        enabled(true);
    } else {
        enabled(false);
    }
}


Wpa::~Wpa()
{
    if (device) delete device;
    if (p2pInterface) delete p2pInterface;
}

void Wpa::connectPeer(const QVariantMap &properties)
{
    QString addr = properties.value("address").toString();
    QString method = properties.value("method").toString();
    QString pin = properties.value("pincode").toString();
    int go_intent = properties.value("go_intent").toInt();
    QString p = QString("%1/Peers/%2").arg(interfacePath).
        arg(addr.remove(":"));
    QDBusObjectPath peer(p);
    QVariantMap args;

    if (!p2pInterface)
        return;

    args["join"] = true;
    if (method == "pin") {
        args["join"] = false;
        if (go_intent == 15) {
            args["authorize_only"] = true;
        } else {
            QStringList params;
            params << "p2p_connect" << properties.value("address").toString()
                   << pin;
            QProcess::execute("/usr/sbin/wpa_cli", params);
            return;
        }
    }

    args["peer"] = qVariantFromValue(peer);
    args["persistent"] = false;
    args["wps_method"] = method;
    args["go_intent"] = go_intent;
    args["pin"] = pin;

    QDBusPendingCallWatcher *watcher;
    watcher = new QDBusPendingCallWatcher(p2pInterface->Connect(args), this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(connectResult(QDBusPendingCallWatcher*)));
}

void Wpa::connectResult(QDBusPendingCallWatcher *watcher)
{
    watcher->deleteLater();
    QDBusPendingReply<int> reply = *watcher;

    if (!reply.isValid()) {
        qDebug() << "Connect Fails: " << reply.error().name();
        qDebug() << reply.error().message();
    }
}

void Wpa::deviceWasFound(const QDBusObjectPath &path)
{
    Peer p(wpa_service, path.path(), QDBusConnection::systemBus());
    QVariantMap properties = p.properties();
    QString deviceName = properties.value("DeviceName").toString();
    QByteArray addr = path.path().split("/").last().toAscii();

    for (int i = 2; i < addr.size(); i+=3)
        addr.insert(i, ':');

    Device dev(addr, deviceName);
    emit deviceFound(dev);
}

void Wpa::disconnectP2P()
{
    QDBusPendingCallWatcher *watcher;
    if (!p2pInterface)
        return;

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
    if (!p2pInterface)
        return;

    watcher = new QDBusPendingCallWatcher(p2pInterface->Find(QVariantMap()),
                                          this);
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
    if (!p2pInterface)
        return;

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
    Q_PID pid;
    bool go = properties.value("role").toString() == "GO";

    if (go) {
        QStringList args;
        args << "server";
        // this method is called twice, that's the reason of
        // this hack
        pid = proc_find("udhcpd");
        if (pid == -1)
            QProcess::execute("/usr/bin/wifi_init.sh", args);
    } else {
        pid = proc_find("udhcpc -i wlan0");
        if (pid == -1)
            QProcess::execute("/usr/bin/wifi_init.sh");
    }

    emit groupStarted(go);
}

void Wpa::groupHasFinished(const QString &ifname,
                           const QString &role)
{
    Q_UNUSED(ifname);
    Q_UNUSED(role);

    emit groupFinished();
}

void Wpa::groupStartResult(QDBusPendingCallWatcher *watcher)
{
    watcher->deleteLater();
    QDBusPendingReply<> reply = *watcher;

    if (!reply.isValid()) {
        qDebug() << "Group Start Fails: " << reply.error().name();
        emit groupStartFails();
    }
}

bool Wpa::isEnabled()
{
    return (wpaPid == -1) ? false : true;
}

void Wpa::setEnabled(bool enable)
{
    if (enable && (wpaPid != -1))
        return;

    if (enable) {
        setupDBus();
        wpaPid = proc_find(wpa_process_name);
        emit enabled(true);
    } else {
        delete device;
        delete p2pInterface;
        device = NULL;
        p2pInterface = NULL;

        kill(wpaPid, SIGKILL);
        wpaPid = -1;
        QProcess::startDetached("/usr/bin/wifi_exit.sh");
        emit enabled(false);
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

    device = new InterfaceDevice(wpa_service, interfacePath,
                              QDBusConnection::systemBus());
    connect(device, SIGNAL(PropertiesChanged(const QVariantMap&)),
            this, SLOT(devicePropertiesChanged(const QVariantMap&)));
    emit status(device->state());

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
    connect(p2pInterface, SIGNAL(GONegotiationFailure(int)), this,
            SLOT(goNegotiationFailure(int)));
    connect(p2pInterface,
            SIGNAL(GONegotiationRequest(const QDBusObjectPath&, int)),
            this, SLOT(goNegotiationRequest(const QDBusObjectPath&, int)));
    connect(p2pInterface,
            SIGNAL(ProvisionDiscoveryPBCRequest(const QDBusObjectPath&)),
            this, SLOT(provisionDiscoveryPBCRequest(const QDBusObjectPath&)));

    wps = new WPS(wpa_service, interfacePath,
               QDBusConnection::systemBus());
    wps->setProcessCredentials(true);

    find();
}

void Wpa::stateChanged(const QStringMap &states)
{
    qDebug() << "states: " << states.keys();
}

void Wpa::startGroup()
{
    QDBusPendingCallWatcher *watcher;
    QVariantMap args;
    args["persistent"] = true;

    if (!p2pInterface)
        return;

    watcher = new QDBusPendingCallWatcher(p2pInterface->GroupAdd(args), this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(groupStartResult(QDBusPendingCallWatcher*)));
}

void Wpa::stopGroup()
{
    QStringList args;

    QProcess::startDetached("/usr/bin/wifi_exit.sh");
    args << "p2p_group_remove" << "wlan0";
    QProcess::execute("/usr/sbin/wpa_cli", args);
}

void Wpa::goNegotiationFailure(int status)
{
    emit connectFails(status);
}

void Wpa::goNegotiationRequest(const QDBusObjectPath &path, int dev_passwd_id)
{
    qDebug() << "goNegotiationRequest";
    qDebug() << "Request: " << path.path();
    qDebug() << "Passwd id " << dev_passwd_id;
}

void Wpa::devicePropertiesChanged(const QVariantMap &properties)
{
    emit status(properties.value("State").toString());
}

QString Wpa::getStatus()
{
    return device->state();
}

void Wpa::setDeviceName(const QString &deviceName)
{
   QVariantMap args;
   args["DeviceName"] = deviceName;

   if (!p2pInterface)
       return;

   p2pInterface->setP2PDeviceProperties(args);
}

void Wpa::provisionDiscoveryPBCRequest(const QDBusObjectPath &peer_object)
{
    Q_UNUSED(peer_object);

    QVariantMap args;
    QDBusPendingCallWatcher *watcher;

    args["Role"] = wps_role;
    args["Type"] = "pbc";

    watcher = new QDBusPendingCallWatcher(
        wps->Start(args), this);
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
            this, SLOT(wpsResult(QDBusPendingCallWatcher*)));
}

void Wpa::wpsResult(QDBusPendingCallWatcher *watcher)
{
    watcher->deleteLater();
    QDBusPendingReply<QVariantMap> reply = *watcher;

    if (!reply.isValid()) {
        qDebug() << "WPS fails: " << reply.error().name();
        qDebug() << reply.error().message();
        return;
    }
}
