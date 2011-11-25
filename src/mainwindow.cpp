#include "mainwindow.h"

#include "keyboard.h"

#include <QDebug>
#include <QLineEdit>
#include <QStringListModel>

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
    devicesModel = NULL;
    wpa = new WPAp2p;

#if !defined(DEBUG)
    QWSServer *qws = QWSServer::instance();
    if (qws)
        qws->setCursorVisible(false);
#endif

    if (wpa->isRunning()) {
        wifiDirectCheckBox->setCheckState(Qt::Checked);
    } else {
        wifiDirectCheckBox->setCheckState(Qt::Unchecked);
        wifiDirectStatusLabel->setText("Disabled");
    }

    buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(pbcRadioButton);
    buttonGroup->addButton(pinRadioButton);

    dynamic_cast<QVBoxLayout *>(layout())->setStretchFactor(scrollArea, 1);
    connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)), this,
            SLOT(focusChanged(QWidget*, QWidget*)));
    connect(wpa, SIGNAL(status(const QString&)), wifiDirectStatusLabel,
            SLOT(setText(const QString&)));
    connect(wpa, SIGNAL(devicesFounded(const QList<Device>&)), this,
            SLOT(devicesFounded(const QList<Device>&)));
    connect(wpa, SIGNAL(deviceUpdate(Device)), this,
            SLOT(deviceUpdate(Device)));
    connect(wpa, SIGNAL(groupStarted()), this,
            SLOT(groupStarted()));
    connect(wpa, SIGNAL(groupStopped()), this,
            SLOT(groupStopped()));
    connect(wpa, SIGNAL(enabled(bool)), this,
            SLOT(setWifiDirectEnabled(bool)));
    connect(intentSlider, SIGNAL(valueChanged(int)), wpa,
            SLOT(setIntent(int)));
    connect(listView, SIGNAL(doubleClicked(const QModelIndex&)), this,
            SLOT(deviceSelected(const QModelIndex&)));
    connect(startGroupButton, SIGNAL(clicked()), wpa,
            SLOT(startGroup()));
    connect(refreshButton, SIGNAL(clicked()), wpa,
            SLOT(scan()));
    connect(channelSlider, SIGNAL(sliderReleased()), this,
            SLOT(channelReleased()));
    wpa->start();
}

MainWindow::~MainWindow()
{
    wpa->stop();
    wpa->wait(2500000); // 2.5s

    delete buttonGroup;
    delete devicesModel;
    delete wpa;
}

void MainWindow::acceptConnectClicked()
{
    bool go = (goCheckBox->checkState() == Qt::Checked) ? true : false;

    if (buttonGroup->checkedButton() == pbcRadioButton)
        wpa->connectPBC(selectedDevice, go, 7);
    else
        wpa->connectPIN(selectedDevice, pinLineEdit->text(), go);

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
        wpa->setChannel(1);
    } else if (value < 9) {
        channelSlider->setValue(6);
        wpa->setChannel(6);
    } else {
        channelSlider->setValue(11);
        wpa->setChannel(11);
    }
}

void MainWindow::devicesFounded(const QList<Device> &devices)
{
    QStringList devs;
    this->devices = devices;
    foreach (Device d, devices) {
        devs << d.value();
    }

    delete devicesModel;
    devicesModel = new QStringListModel(devs);
    listView->setModel(devicesModel);
}

void MainWindow::deviceSelected(const QModelIndex &index)
{
    selectedDevice = index.data().toString();
    int i = selectedDevice.indexOf("-");
    if (i != -1)
        selectedDevice = selectedDevice.mid(i + 2); // "- "

    pinLineEdit->clear();
    pbcRadioButton->setChecked(true);
    stackedWidget->setCurrentWidget(connectPage);
}

void MainWindow::deviceUpdate(Device device)
{
    int index = 0;
    QStringList devs;
    foreach (Device d, devices) {
        if (d.address() == device.address()) {
            QModelIndex i = listView->model()->index(index, 0);
            listView->model()->setData(i, device.value());
            break;
        }
        index++;
    }
}

void MainWindow::enableStateChanged(int state)
{
    if (state == Qt::Checked) {
        wpa->setEnabled(true);
    } else {
        wpa->setEnabled(false);
        devices.clear();
        QStringListModel *model =
            qobject_cast<QStringListModel*>(listView->model());
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
}

void MainWindow::groupStopped()
{
    startGroupButton->setText("Start Group");
}

void MainWindow::setName()
{
    QString name = nameLineEdit->text();
    wpa->setName(name);
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
