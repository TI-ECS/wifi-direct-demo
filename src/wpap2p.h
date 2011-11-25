#ifndef _WPAP2P_H_
#define _WPAP2P_H_

#include "device.h"

#if defined(DEBUG)
#include <QFile>
#endif

#include <QHash>
#include <QMutex>
#include <QObject>
#include <QProcess>
#include <QQueue>
#include <QThread>
#include <QVariant>


enum ACTIONS {
    CHANGE_CHANNEL,
    CHANGE_INTENT,
    GETTING_STATUS,
    GETTING_PEER_INFORMATION,
    NONE,
    SCANNING,
    SCAN_RESULT,
    START_GROUP,
    STOP_GROUP
};

typedef struct ActionValue_ {
    ACTIONS action;
    QVariant value;
} ActionValue;

class WPAp2p : public QThread
{
Q_OBJECT

public:
    WPAp2p(QObject *parent = 0);
    virtual ~WPAp2p();
    bool isRunning() { return (WPAPid == -1) ? false : true; }
    void run();

public slots:
    void scan();
    void setChannel(int value);
    void setEnabled(bool state);
    void setIntent(int value);
    void start(Priority priority = InheritPriority);
    void startGroup();

private slots:
    void getPeer();
    void getPeers();
    void readWPAStandartOutput();

signals:
    void devicesFounded(const QList<Device> &devices);
    void deviceUpdate(const Device &device);
    void enabled(bool started);
    void groupStarted();
    void groupStopped();
    void status(const QString &status);

private:
    ACTIONS currentAction;

#if defined(DEBUG)
    QFile logFile;
#endif

    QHash<QString, Device> devices;
    QMutex mutex;
    QProcess WPAProcess;
    QQueue<ActionValue> actionsQueue;
    QString currentDevice;
    bool hasGroup;
    qint64 WPAPid;
};


#endif /* _WPAP2P_H_ */
