#ifndef _WPAP2P_H_
#define _WPAP2P_H_

#include <QMutex>
#include <QObject>
#include <QProcess>
#include <QThread>
#include <QQueue>


enum ACTIONS {
    CHANGE_CHANNEL,
    CHANGE_INTENT,
    GETTING_STATUS,
    NONE,
    SCANNING,
    SCAN_RESULT,
    START_GROUP,
    STOP_GROUP
};

typedef struct ActionValue_ {
    ACTIONS action;
    int value;
} ActionValue;

class WPAp2p : public QThread
{
Q_OBJECT

public:
    WPAp2p(QObject *parent = 0);
    virtual ~WPAp2p();
    void run();
    bool isRunning() { return (WPAPid == -1) ? false : true; }

public slots:
    void scan();
    void setChannel(int value);
    void setEnabled(bool state);
    void setIntent(int value);
    void start(Priority priority = InheritPriority);
    void startGroup();

private slots:
    void getPeers();
    void readWPAStandartOutput();

signals:
    void devicesFounded(const QStringList &devices);
    void groupStarted();
    void groupStopped();
    void status(const QString &status);
    void enabled(bool started);

private:
    bool hasGroup;
    qint64 WPAPid;
    QMutex mutex;
    QProcess WPAProcess;
    QQueue<ActionValue> actionsQueue;
    ACTIONS currentAction;
};


#endif /* _WPAP2P_H_ */
