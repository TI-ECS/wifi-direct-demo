#include "wpap2p.h"

#include <QDebug>
#include <QDir>
#include <QTimer>
#include <signal.h>
#include <sys/types.h>

#define CREATE_GROUP "p2p_group_add\n"
#define GET_STATUS "status\n"
#define P2P_FIND "p2p_find %1\n"
#define REMOVE_GROUP "p2p_group_remove %1\n"
#define SET_COMMAND "set %1 %2\n"
#define WPA_PROCESS_NAME "wpa_supplicant"
#define TIMEOUT 20000           // 20s


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
    :QObject(parent),
     WPAProcess(0)
{
    hasGroup = false;
    currentAction = NONE;
    connect(&WPAProcess, SIGNAL(readyReadStandardOutput()),
            this, SLOT(readWPAStandartOutput()));

    WPAPid = proc_find(WPA_PROCESS_NAME);
}

WPAp2p::~WPAp2p()
{
    WPAProcess.close();
    WPAProcess.kill();
}

void WPAp2p::getPeers()
{
    emit devicesFounded(QStringList());
}

void WPAp2p::readWPAStandartOutput()
{
    if (currentAction == NONE)
        return;

    QString value(WPAProcess.read(WPAProcess.bytesAvailable()));
    int index;

    switch (currentAction) {
    case GETTING_STATUS:
        if ((index = value.indexOf("wpa_state=")) > -1) {
            emit status(value.mid(index + 10, value.indexOf("\n", index) - index - 10));
            currentAction = NONE;
        }
        break;
    case START_GROUP:
        if (value.contains("OK")) {
            emit groupStarted();
            hasGroup = true;
            WPAProcess.write(GET_STATUS);
            currentAction = GETTING_STATUS;
        }
        break;
    case STOP_GROUP:
        if (value.contains("OK")) {
            emit groupStopped();
            hasGroup = false;
            WPAProcess.write(GET_STATUS);
            currentAction = GETTING_STATUS;
        }
        break;
    case SCANNING:
    case CHANGE_INTENT:
        if (value.contains("OK"))
            currentAction = NONE;
    case CHANGE_CHANNEL:
    default: break;
    }
}

void WPAp2p::scan()
{
    WPAProcess.write(QString(P2P_FIND).arg(TIMEOUT).
                     toAscii());
    QTimer::singleShot(TIMEOUT, this, SLOT(getPeers()));
}

void WPAp2p::setChannel(int value)
{
    qDebug() << "channel: " << value;
}

void WPAp2p::setIntent(int value)
{
    WPAProcess.write(QString(SET_COMMAND).arg("p2p_go_intent").
                     arg(value).toAscii());
}

bool WPAp2p::start()
{
   WPAProcess.start("wpa_cli");
    if (!WPAProcess.waitForStarted(3000))
        return false;

    WPAProcess.write(QString(P2P_FIND).arg(TIMEOUT).
                     toAscii());
    WPAProcess.write(GET_STATUS);
    currentAction = GETTING_STATUS;
    return true;
}

void WPAp2p::startGroup()
{
    if (hasGroup) {
        WPAProcess.write(QString(REMOVE_GROUP).
                                 arg("wlan0").toAscii());
        currentAction = STOP_GROUP;
    } else {
        WPAProcess.write(CREATE_GROUP);
        currentAction = START_GROUP;
    }
}

void WPAp2p::setEnabled(bool state)
{
    if (state) {
        if (WPAPid != -1)
            return;
        if (QProcess::startDetached(WPA_PROCESS_NAME,
                                    QStringList() << "d" << "-Dnl80211"
                                    << "-c/etc/wpa_supplicant.conf" << "-iwlan0"
                                    << "-B", QDir::rootPath(), &WPAPid))
            emit enabled(true);
        else
            emit enabled(false);
    } else {
        if (kill(WPAPid, SIGKILL) != -1)
            emit enabled(false);
    }
}
