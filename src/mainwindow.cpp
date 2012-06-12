#include "mainwindow.h"

#include "devicelistdelegate.h"
#include "deviceslistmodel.h"
#include "keyboard.h"
#include "wpa.h"

#include <QDebug>
#include <QLineEdit>
#include <QMessageBox>

#if !defined(DEBUG)
#include <QWSServer>
#endif

void MainWindow::buttonsEnabled(bool enable)
{
    if (enable) {
        startGroupButton->setEnabled(true);
        disconnectButton->setEnabled(true);
    } else {
        startGroupButton->setEnabled(false);
        disconnectButton->setEnabled(false);
    }
}

MainWindow::MainWindow(QWidget *parent)
    :QWidget(parent)
{
    setupUi(this);
    keyboard = new Keyboard;
    keyboard->setVisible(false);
    layout()->addWidget(keyboard);
    devicesModel = new DevicesListModel;
    listView->setModel(devicesModel);
    deviceDelegate = new DeviceListDelegate;
    listView->setItemDelegate(deviceDelegate);

#if !defined(DEBUG)
    QWSServer *qws = QWSServer::instance();
    if (qws)
        qws->setCursorVisible(false);
#endif

    buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(pbcRadioButton);
    buttonGroup->addButton(joinGroupRadioButton);
    buttonGroup->addButton(pinRadioButton);
    buttonsEnabled(false);

    wpa = new Wpa;

    dynamic_cast<QVBoxLayout *>(layout())->setStretchFactor(scrollArea, 1);
    connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)), this,
            SLOT(focusChanged(QWidget*, QWidget*)));
    connect(wpa, SIGNAL(status(const QString&)), this,
            SLOT(statusChanged(const QString&)));
    connect(wpa, SIGNAL(deviceFound(Device&)), devicesModel,
            SLOT(addDevice(Device&)));
    connect(wpa, SIGNAL(connectFails(int)), this,
            SLOT(connectionFails(int)));
    connect(wpa, SIGNAL(groupStarted(bool)), this,
            SLOT(groupStarted(bool)));
    connect(wpa, SIGNAL(groupFinished()), this,
            SLOT(groupStopped()));
    connect(wpa, SIGNAL(enabled(bool)), this,
            SLOT(setWifiDirectEnabled(bool)));
    connect(listView, SIGNAL(doubleClicked(const QModelIndex&)), this,
            SLOT(deviceSelected(const QModelIndex&)));
    connect(startGroupButton, SIGNAL(clicked()), this,
            SLOT(startGroupClicked()));
    connect(disconnectButton, SIGNAL(clicked()), this,
            SLOT(disconnectClicked()));
    // connect(intentSlider, SIGNAL(valueChanged(int)), wpa,
    //         SLOT(setIntent(int)));
    // connect(channelSlider, SIGNAL(sliderReleased()), this,
    //         SLOT(channelReleased()));

    if (wpa->isEnabled()) {
        wifiDirectCheckBox->setCheckState(Qt::Checked);
        wpa->getPeers();

        statusChanged(wpa->getStatus());
    }
}

MainWindow::~MainWindow()
{
    delete wpa;
    delete buttonGroup;
    delete devicesModel;
    delete deviceDelegate;
 }

void MainWindow::acceptConnectClicked()
{
    bool go = (goCheckBox->checkState() == Qt::Checked) ? true : false;
    QVariantMap properties;
    properties["address"] = selectedDevice;
    properties["pincode"] = pinLineEdit->text();
    properties["go_intent"] = (go) ? 15 : 0;

    if (buttonGroup->checkedButton() == joinGroupRadioButton) {
        properties["join"] = true;
        properties["method"] = "pbc";
    } else {
        properties["join"] = false;
        properties["method"] = (buttonGroup->checkedButton() == pbcRadioButton) ?
            "pbc" : "pin";
    }

    wpa->connectPeer(properties);

    stackedWidget->setCurrentWidget(mainPage);
}

void MainWindow::backClicked()
{
    stackedWidget->setCurrentWidget(mainPage);
}

void MainWindow::cancelConnectClicked()
{
    stackedWidget->setCurrentWidget(mainPage);
}

void MainWindow::channelReleased()
{
    int value = channelSlider->value();
    if (value < 4) {
        channelSlider->setValue(1);
        // wpa->setChannel(1);
    } else if (value < 9) {
        channelSlider->setValue(6);
        // wpa->setChannel(6);
    } else {
        channelSlider->setValue(11);
        // wpa->setChannel(11);
    }
}

void MainWindow::connectionFails(int status)
{
    QMessageBox::warning(this, "Failed",
                         QString("Connection failed with status: %1").
                         arg(status));
}

void MainWindow::deviceSelected(const QModelIndex &index)
{
    selectedDevice = index.data(Qt::UserRole).
        value<Device *>()->address();

    pinLineEdit->clear();
    pbcRadioButton->setChecked(true);
    stackedWidget->setCurrentWidget(connectPage);
}

void MainWindow::enableStateChanged(int state)
{
    if (state == Qt::Checked) {
        buttonsEnabled(true);

        wpa->setEnabled(true);
    } else {
        buttonsEnabled(false);

        DevicesListModel *model =
            qobject_cast<DevicesListModel *>(listView->model());
        model->removeRows(0, model->rowCount());
    }
}

void MainWindow::disconnectClicked()
{
    QProcess::startDetached("/usr/bin/wifi_exit.sh");
    wpa->disconnectP2P();
}

void MainWindow::exitClicked()
{
    close();
}

void MainWindow::focusChanged(QWidget *old, QWidget *now)
{
    Q_UNUSED(old);

    if (!now || !now->objectName().size())
        return;

    if (now->inherits("QLineEdit")) {
        keyboard->setVisible(true);
        scrollArea->ensureWidgetVisible(now);
    } else
        keyboard->setVisible(false);
}

void MainWindow::groupStarted(bool go)
{
    disconnectButton->setEnabled(true);
    if (go)
        startGroupButton->setText("Stop Group");
}

void MainWindow::groupStopped()
{
    startGroupButton->setText("Start Group");
}

void MainWindow::setName()
{
    QString name = nameLineEdit->text();
    wpa->setDeviceName(name);
}

void MainWindow::settingsClicked()
{
    backButton->setFocus(Qt::OtherFocusReason);
    stackedWidget->setCurrentWidget(settingsPage);
}

void MainWindow::setWifiDirectEnabled(bool state)
{
    if (state) {
        buttonsEnabled(true);
        wifiDirectCheckBox->setCheckState(Qt::Checked);
    } else {
        buttonsEnabled(false);

        wifiDirectCheckBox->setCheckState(Qt::Unchecked);
        wifiDirectStatusLabel->setText("Disabled");
        startGroupButton->setText("Start Group");
    }
}

void MainWindow::showPinCode(const QString &pin)
{
    QMessageBox::information(this, "PIN CODE", pin);
}

void MainWindow::startGroupClicked()
{
    if (startGroupButton->text() == "Start Group") {
        wpa->startGroup();
    } else {
        wpa->stopGroup();
    }
}

void MainWindow::statusChanged(const QString &state)
{
    if (state == "completed") {
        startGroupButton->setText("Stop Group");
        disconnectButton->setEnabled(true);
    } else if (state.isEmpty() || (state == "inactive")
               || (state == "scanning")) {
        startGroupButton->setText("Start Group");
        disconnectButton->setEnabled(false);
    }

    wifiDirectStatusLabel->setText(state);
}
