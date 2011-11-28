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
#include <QSharedPointer>
#include <QThread>
#include <QVariant>


enum ACTIONS {
    CHANGE_CHANNEL,
    CHANGE_INTENT,
    CONNECT,
    GETTING_STATUS,
    GETTING_PEER_INFORMATION,
    NONE,
    SETTING_NAME,
    SCANNING,
    SCAN_RESULT,
    START_GROUP,
    STOP_GROUP
};

enum WPS_METHOD {
    PBC,
    PBC_GO,
    PIN,
    PIN_GO
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
    void connectPBC(const QString &device, bool go, int intent);
    void connectPIN(const QString &device, const QString &pin, bool go);
    void scan();
    void setChannel(int value);
    void setEnabled(bool state);
    void setIntent(int value);
    void setName(const QString &value);
    void start(Priority priority = InheritPriority);
    void startGroup();
    void stop();

private slots:
    void getPeer();
    void getPeers();
    void readWPAStandartOutput();

protected:
    int exec();

signals:
    void connectCommandFinished();
    void devicesFounded(const QList<QSharedPointer<Device> > &devices);
    void deviceUpdate(Device *device);
    void enabled(bool started);
    void groupStarted();
    void groupStopped();
    void pinCode(const QString &pin);
    void status(const QString &status);

private:
    ACTIONS currentAction;
    WPS_METHOD wpsMethod;

#if defined(DEBUG)
    QFile logFile;
#endif

    QHash<QString, QSharedPointer<Device> > devices;
    QMutex mutex;
    QProcess WPAProcess;
    QQueue<ActionValue> actionsQueue;
    QString currentDevice;
    bool active, hasGroup;
    qint64 WPAPid;
};


#endif /* _WPAP2P_H_ */
