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

public slots:
    void setChannel(int value);
    void setIntent(int value);
    void startGroup();
    void scan();

private slots:
    void readWPAStandartOutput();

signals:
    void groupStarted();
    void groupStopped();
    void status(const QString &status);

private:
    bool hasGroup;
    QProcess wpaProcess;
    ACTIONS currentAction;
};


#endif /* _WPAP2P_H_ */
