#include "wpap2p.h"

#include <QDebug>

#define CREATE_GROUP "p2p_group_add\n"
#define GET_STATUS "status\n"
#define P2P_FIND "p2p_find %1\n"
#define REMOVE_GROUP "p2p_group_remove %1\n"
#define SET_COMMAND "set %1 %2\n"
#define TIMEOUT 10000           // 10s


WPAp2p::WPAp2p(QObject *parent)
    :QObject(parent),
     wpaProcess(0)
{
    hasGroup = false;
    currentAction = NONE;
    connect(&wpaProcess, SIGNAL(readyReadStandardOutput()),
            this, SLOT(readWPAStandartOutput()));
}

WPAp2p::~WPAp2p()
{
}

void WPAp2p::readWPAStandartOutput()
{
    if (currentAction == NONE)
        return;

    QString value(wpaProcess.read(wpaProcess.bytesAvailable()));
    int index;

    switch (currentAction) {
    case GETTING_STATUS:
        if ((index = value.indexOf("wpa_state=")) > -1) {
            emit status(value.mid(index + 10, value.indexOf("\n", index) -  10));
            currentAction = NONE;
        }
        break;
    case START_GROUP:
        if (value.contains("OK")) {
            emit groupStarted();
            hasGroup = true;
            wpaProcess.write(GET_STATUS);
            currentAction = GETTING_STATUS;
        }
        break;
    case STOP_GROUP:
        if (value.contains("OK")) {
            emit groupStopped();
            hasGroup = false;
            wpaProcess.write(GET_STATUS);
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
    wpaProcess.write(QString(P2P_FIND).arg(TIMEOUT).
                     toAscii());
}

void WPAp2p::setChannel(int value)
{
    qDebug() << "channel: " << value;
}

void WPAp2p::setIntent(int value)
{
    wpaProcess.write(QString(SET_COMMAND).arg("p2p_go_intent").
                     arg(value).toAscii());
}

bool WPAp2p::start()
{
   wpaProcess.start("wpa_cli");
    if (!wpaProcess.waitForStarted(3000))
        return false;

    wpaProcess.write(QString(P2P_FIND).arg(TIMEOUT).
                     toAscii());
    wpaProcess.write(GET_STATUS);
    currentAction = GETTING_STATUS;
    return true;
}

void WPAp2p::startGroup()
{
    if (hasGroup) {
        wpaProcess.write(QString(REMOVE_GROUP).
                                 arg("wlan0").toAscii());
        currentAction = STOP_GROUP;
    } else {
        wpaProcess.write(CREATE_GROUP);
        currentAction = START_GROUP;
    }
}
