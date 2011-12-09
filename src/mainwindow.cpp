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

    wpa = new Wpa;
    dynamic_cast<QVBoxLayout *>(layout())->setStretchFactor(scrollArea, 1);
    connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)), this,
            SLOT(focusChanged(QWidget*, QWidget*)));
    connect(wpa, SIGNAL(status(const QString&)), wifiDirectStatusLabel,
            SLOT(setText(const QString&)));
    connect(wpa, SIGNAL(deviceFound(Device&)), devicesModel,
            SLOT(addDevice(Device&)));
    connect(wpa, SIGNAL(groupStarted()), this,
            SLOT(groupStarted()));
    connect(wpa, SIGNAL(groupFinished()), this,
            SLOT(groupStopped()));
    connect(wpa, SIGNAL(enabled(bool)), this,
            SLOT(setWifiDirectEnabled(bool)));
    connect(listView, SIGNAL(doubleClicked(const QModelIndex&)), this,
            SLOT(deviceSelected(const QModelIndex&)));
    connect(startGroupButton, SIGNAL(clicked()), this,
            SLOT(startGroupClicked()));
    // connect(wpa, SIGNAL(pinCode(const QString&)), this,
    //         SLOT(showPinCode(const QString&)));
    // connect(intentSlider, SIGNAL(valueChanged(int)), wpa,
    //         SLOT(setIntent(int)));
    // connect(refreshButton, SIGNAL(clicked()), wpa,
    //         SLOT(scan()));
    // connect(channelSlider, SIGNAL(sliderReleased()), this,
    //         SLOT(channelReleased()));
    wpa->getPeers();
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

void MainWindow::channelReleased(){
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
        wpa->setEnabled(true);
    } else {
        wpa->setEnabled(false);
        DevicesListModel *model =
            qobject_cast<DevicesListModel *>(listView->model());
        model->removeRows(0, model->rowCount());
    }
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

void MainWindow::groupStarted()
{
    startGroupButton->setText("Stop Group");

    // As we're the GO, init udhcpd
    QStringList args;
    args << "server";
    QProcess::startDetached("/usr/bin/wifi_init.sh", args);
}

void MainWindow::groupStopped()
{
    startGroupButton->setText("Start Group");
}

void MainWindow::setName()
{
    QString name = nameLineEdit->text();
    // wpa->setName(name);
}

void MainWindow::settingsClicked()
{
    backButton->setFocus(Qt::OtherFocusReason);
    stackedWidget->setCurrentWidget(settingsPage);
}

void MainWindow::setWifiDirectEnabled(bool state)
{
    if (state) {
        wifiDirectCheckBox->setCheckState(Qt::Checked);
    } else {
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
        // TODO: Remove group
    }
}
