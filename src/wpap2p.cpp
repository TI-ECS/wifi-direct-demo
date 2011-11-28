#include "wpap2p.h"

#include <QDebug>
#include <QDir>
#include <QTimer>
#include <signal.h>
#include <sys/types.h>

#define CREATE_CONNECTION "p2p_connect %1 %2 %3 %4\n"
#define CREATE_GROUP "p2p_group_add\n"
#define GET_PEER "p2p_peer %1\n"
#define GET_PEERS "p2p_peers\n"
#define GET_STATUS "status\n"
#define P2P_FIND "p2p_find\n"
#define REMOVE_GROUP "p2p_group_remove %1\n"
#define SET_CHANNEL "p2p_set listen_channel %1\n"
#define SET_COMMAND "set %1 %2\n"
#define WPA_PROCESS_NAME "wpa_supplicant"
#define TIMEOUT 8000           // 8s


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

WPAp2p::WPAp2p(QObject *parent)
    :QThread(parent),
     WPAProcess(0)
{
    active = hasGroup = false;
    currentAction = NONE;
    connect(&WPAProcess, SIGNAL(readyReadStandardOutput()),
            this, SLOT(readWPAStandartOutput()));

    WPAPid = proc_find(WPA_PROCESS_NAME);

#if defined(DEBUG)
    logFile.setFileName("/home/root/wifi-direct.log");
    logFile.open(QIODevice::WriteOnly | QIODevice::Text);
#endif
}

WPAp2p::~WPAp2p()
{
#if defined(DEBUG)
    logFile.close();
#endif

    WPAProcess.close();
    WPAProcess.kill();
}

void WPAp2p::connectPBC(const QString &device, bool go, int intent)
{
    QString connection;

    if (go) {
        QString go_intent = QString("go_intent=%1").arg(intent);
        connection = QString(CREATE_CONNECTION).arg(device).arg("pbc").
            arg("auth").arg(go_intent);
        wpsMethod = PBC_GO;
    } else {
        connection = QString(CREATE_CONNECTION).arg(device).arg("pbc");
        wpsMethod = PBC;
    }

    mutex.lock();
    ActionValue action = {CONNECT, connection};
    actionsQueue.enqueue(action);
    mutex.unlock();
}

void WPAp2p::connectPIN(const QString &device, const QString &pin, bool go)
{
    QString connection;

    if (go) {
        connection = QString(CREATE_CONNECTION).arg(device).arg("pin").
            arg("auth");
        wpsMethod = PIN_GO;
    } else {
        connection = QString(CREATE_CONNECTION).arg(device).arg("pin").
            arg(pin);
        wpsMethod = PIN;
    }

    mutex.lock();
    ActionValue action = {CONNECT, connection};
    actionsQueue.enqueue(action);
    mutex.unlock();
}

int WPAp2p::exec()
{
    while (active) {
        sleep(2.5);
        mutex.lock();
        if (currentAction == NONE) {
            if (!actionsQueue.isEmpty()) {
                ActionValue action = actionsQueue.dequeue();
                currentAction = action.action;

                switch (action.action) {
                case CHANGE_CHANNEL:
                    WPAProcess.write(QString(SET_CHANNEL).
                                     arg(action.value.toInt()).toAscii());
                    break;
                case CHANGE_INTENT:
                    WPAProcess.write(QString(SET_COMMAND).arg("p2p_go_intent").
                                     arg(action.value.toInt()).toAscii());
                    break;
                case CONNECT:
                    WPAProcess.write(action.value.toString().toAscii());
                    break;
                case GETTING_PEER_INFORMATION:
                {
                    QString d(action.value.toString());
                    currentDevice = d;
                    WPAProcess.write(QString(GET_PEER).
                                     arg(d).toAscii());
                }
                    break;
                case GETTING_STATUS:
                    WPAProcess.write(GET_STATUS);
                    break;
                case SCANNING:
                    WPAProcess.write(QString(P2P_FIND).toAscii());
                    break;
                case SCAN_RESULT:
                    WPAProcess.write(GET_PEERS);
                    break;
                case SETTING_NAME:
                    WPAProcess.write(QString(SET_COMMAND).arg("device_name").
                                     arg(action.value.toString()).toAscii());
                    break;
                case START_GROUP:
                    WPAProcess.write(CREATE_GROUP);
                    break;
                case STOP_GROUP:
                    WPAProcess.write(QString(REMOVE_GROUP).
                                     arg("wlan0").toAscii());
                    break;
                default:
                    break;
                }
            }
        }
        mutex.unlock();
    }

    return 0;
}

void WPAp2p::getPeer()
{
    if (WPAPid == -1) return;

    mutex.lock();
    ActionValue action = {SCAN_RESULT, 0};
    actionsQueue.enqueue(action);
    mutex.unlock();
}

void WPAp2p::getPeers()
{
    if (WPAPid == -1) return;

    mutex.lock();
    ActionValue action = {SCAN_RESULT, 0};
    actionsQueue.enqueue(action);
    mutex.unlock();
}

void WPAp2p::readWPAStandartOutput()
{
    QString value(WPAProcess.read(WPAProcess.bytesAvailable()));
#if defined(DEBUG)
    logFile.write(QString("Current action: %1").
                  arg(currentAction).toAscii());
    logFile.write(value.toAscii());
#endif

    if (currentAction == NONE)
        return;

    ActionValue actionStatus = {GETTING_STATUS, 0};
    int index;

    mutex.lock();
    switch (currentAction) {
    case GETTING_STATUS:
        if ((index = value.indexOf("wpa_state=")) > -1)
            emit status(value.mid(index + 10, value.indexOf("\n", index)
                                  - index - 10));
        else
            return;
        break;
    case GETTING_PEER_INFORMATION:
    {
        static QString buf;
        buf.append(value);
        if (buf.endsWith("> ")) {
            devices[currentDevice].data()->setValues(buf);
            emit deviceUpdate(devices[currentDevice].data());
            buf.clear();
        } else {
            goto end;
        }
    }
        break;
    case START_GROUP:
        if (value.contains("OK")) {
            emit groupStarted();
            hasGroup = true;
            actionsQueue.enqueue(actionStatus);
        }
        break;
    case STOP_GROUP:
        if (value.contains("OK")) {
            emit groupStopped();
            hasGroup = false;
            actionsQueue.enqueue(actionStatus);
        }
        break;
    case SCANNING:
        if (value.contains("FAIL"))
            qDebug() << "Scanning fails";
        actionsQueue.enqueue(actionStatus);
        break;
    case SETTING_NAME:
        if (value.contains("FAIL"))
            qDebug() << "Setting name fails";
        break;
    case SCAN_RESULT:
    {
        QString buffer;
        buffer.append(value);
        if (buffer.endsWith("> ")) {
            QStringList devs = buffer.split("\n");
            foreach (const QString &dev, devs) {
                if (dev.startsWith(">"))
                    continue;
                devices[dev] = QSharedPointer<Device>(new Device(dev));
                ActionValue action = {GETTING_PEER_INFORMATION, dev};
                actionsQueue.enqueue(action);
            }
            buffer.clear();
            if (devices.size())
                emit devicesFounded(devices.values());
        } else {
            goto end;
        }
    }
        break;
    case CONNECT:
    {
        QString buffer;
        buffer.append(value);
        if (buffer.endsWith("> ")) {
            if (wpsMethod == PIN_GO) {
                QString pin = buffer.split("\n").at(0);
                emit pinCode(pin);
            }
            emit connectCommandFinished();
            buffer.clear();
        } else
            goto end;
    }
        break;
    case CHANGE_INTENT:
        if (value.contains("FAIL"))
            qDebug() << "Change intent fails";
    case CHANGE_CHANNEL:
        if (value.contains("FAIL"))
            qDebug() << "Setting channel fails";
    default: break;
    }

    currentAction = NONE;

end:
    mutex.unlock();
}

void WPAp2p::run()
{
    active = true;
    exec();

    QThread::exit();
}

void WPAp2p::scan()
{
    if (WPAPid == -1) return;

    mutex.lock();
    ActionValue action = {SCANNING, 0};
    actionsQueue.enqueue(action);
    QTimer::singleShot(TIMEOUT, this, SLOT(getPeers()));
    mutex.unlock();
}

void WPAp2p::setChannel(int value)
{
    if (WPAPid == -1) return;

    mutex.lock();
    ActionValue action = {CHANGE_CHANNEL, value};
    actionsQueue.enqueue(action);
    mutex.unlock();
}

void WPAp2p::setIntent(int value)
{
    if (WPAPid == -1) return;

    mutex.lock();
    ActionValue action = {CHANGE_INTENT, value};
    actionsQueue.enqueue(action);
    mutex.unlock();
}

void WPAp2p::start(Priority priority)
{
    if (WPAPid != -1) {

        WPAProcess.start("/usr/sbin/wpa_cli");
        if (!WPAProcess.waitForStarted(-1))
            return;

        scan();
    } else {
        currentAction = NONE;
    }

    QThread::start(priority);
}

void WPAp2p::startGroup()
{
    if (WPAPid == -1) return;

    ActionValue action;
    if (hasGroup)
        action.action = STOP_GROUP;
    else
        action.action = START_GROUP;

    mutex.lock();
    actionsQueue.enqueue(action);
    mutex.unlock();
}

void WPAp2p::setEnabled(bool state)
{
    if (state) {
        if (WPAPid != -1) return;

        if (QProcess::startDetached(WPA_PROCESS_NAME,
                                    QStringList() << "d" << "-Dnl80211"
                                    << "-c/etc/wpa_supplicant.conf" << "-iwlan0"
                                    << "-B", QDir::rootPath(), &WPAPid)) {
            WPAPid += 1;        // It's really weird, but startDetached is
                                // it's always returning the pid - 1.
            this->sleep(6);     // waiting the wpa_cli reconnects.
            WPAProcess.start("/usr/sbin/wpa_cli");
            if (!WPAProcess.waitForStarted(-1))
                return;

            ActionValue actionStatus = {GETTING_STATUS, 0};
            actionsQueue.enqueue(actionStatus);
            emit enabled(true);
        } else {
            emit enabled(false);
        }
    } else {
        WPAProcess.close();
        if (WPAPid == -1)
            return;
        if (kill(WPAPid, SIGKILL) != -1) {
            WPAPid = -1;
            emit enabled(false);
            actionsQueue.clear();
            currentAction = NONE;
        }
    }
}

void WPAp2p::setName(const QString &value)
{
    if (WPAPid == -1) return;

    mutex.lock();

    ActionValue action = {SETTING_NAME, value};
    actionsQueue.enqueue(action);

    mutex.unlock();
}

void WPAp2p::stop()
{
    mutex.lock();
    active = false;
    mutex.unlock();
}
