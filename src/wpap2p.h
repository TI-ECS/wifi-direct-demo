#ifndef _WPAP2P_H_
#define _WPAP2P_H_

#include <QObject>
#include <QProcess>


enum ACTIONS {
    CHANGE_CHANNEL,
    CHANGE_INTENT,
    GETTING_STATUS,
    NONE,
    SCANNING,
    START_GROUP,
    STOP_GROUP
};

class WPAp2p : public QObject
{
Q_OBJECT

public:
    WPAp2p(QObject *parent = 0);
    virtual ~WPAp2p();
    bool start();
    bool isRunning() { return (WPAPid == -1) ? false : true; }

public slots:
    void scan();
    void setChannel(int value);
    void setIntent(int value);
    void startGroup();
    void setEnabled(bool state);

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
    Q_PID WPAPid;
    QProcess WPAProcess;
    ACTIONS currentAction;
};


#endif /* _WPAP2P_H_ */
