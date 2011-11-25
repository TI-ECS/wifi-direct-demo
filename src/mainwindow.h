#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include "device.h"
#include "keyboard.h"
#include "ui_mainwindow.h"
#include "wpap2p.h"

class QButtonGroup;
class QStringListModel;

class MainWindow : public QWidget, public Ui::MainWindow
{
Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

private slots:
    void channelReleased();
    void focusChanged(QWidget *old, QWidget *now);
    void groupStarted();
    void groupStopped();
    void setWifiDirectEnabled(bool state);

public slots:
    void acceptConnectClicked();
    void backClicked();
    void cancelConnectClicked();
    void devicesFounded(const QList<Device> &devices);
    void deviceUpdate(Device device);
    void enableStateChanged(int state);
    void exitClicked();
    void settingsClicked();

private slots:
    void deviceSelected(const QModelIndex &index);

private:
    Keyboard *keyboard;
    QButtonGroup *buttonGroup;
    QList<Device> devices;
    QStringListModel *devicesModel;
    WPAp2p *wpa;
};


#endif /* _MAINWINDOW_H_ */
