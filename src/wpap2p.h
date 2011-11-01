#ifndef _WPAP2P_H_
#define _WPAP2P_H_

#if defined(DEBUG)
#include <QFile>
#endif

#include <QMutex>
#include <QObject>
#include <QProcess>
#include <QQueue>
#include <QThread>


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
    void getPeers();
    void readWPAStandartOutput();

signals:
    void devicesFounded(const QStringList &devices);
    void enabled(bool started);
    void groupStarted();
    void groupStopped();
    void status(const QString &status);

private:
    ACTIONS currentAction;

#if defined(DEBUG)
    QFile logFile;
#endif

    QMutex mutex;
    QProcess WPAProcess;
    QQueue<ActionValue> actionsQueue;
    bool hasGroup;
    qint64 WPAPid;
};


#endif /* _WPAP2P_H_ */
