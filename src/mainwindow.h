#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include "device.h"
#include "keyboard.h"
#include "ui_mainwindow.h"

class DeviceListDelegate;
class DevicesListModel;
class FiW1Wpa_supplicant1InterfaceP2PDeviceInterface;
class QButtonGroup;
class QStringListModel;
class Wpa;

class MainWindow : public QWidget, public Ui::MainWindow
{
Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

private slots:
    void channelReleased();
    void connectionFails(int status);
    void focusChanged(QWidget *old, QWidget *now);
    void groupStarted(bool go);
    void groupStopped();
    void setWifiDirectEnabled(bool state);
    void startGroupClicked();
    void statusChanged(const QString &state);

public slots:
    void acceptConnectClicked();
    void backClicked();
    void cancelConnectClicked();
    void disconnectClicked();
    void enableStateChanged(int state);
    void exitClicked();
    void setName();
    void settingsClicked();

private slots:
    void deviceSelected(const QModelIndex &index);

private:
    void buttonsEnabled(bool enable);
    DeviceListDelegate *deviceDelegate;
    DevicesListModel *devicesModel;
    FiW1Wpa_supplicant1InterfaceP2PDeviceInterface *p2pInterface;
    Keyboard *keyboard;
    QButtonGroup *buttonGroup;
    QString selectedDevice;
    Wpa *wpa;
};


#endif /* _MAINWINDOW_H_ */
