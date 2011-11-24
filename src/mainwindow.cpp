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
    connect(wpa, SIGNAL(devicesFounded(const QStringList&)), this,
            SLOT(devicesFounded(const QStringList&)));
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
    wpa->terminate();

    delete buttonGroup;
    delete devicesModel;
    delete wpa;
}

void MainWindow::acceptConnectClicked()
{
    qDebug() << "Connect";
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

void MainWindow::devicesFounded(const QStringList &devices)
{
    delete devicesModel;
    devicesModel = new QStringListModel(devices);
    listView->setModel(devicesModel);
}

void MainWindow::deviceSelected(const QModelIndex &index)
{
    pinLineEdit->clear();
    pbcRadioButton->setChecked(true);
    stackedWidget->setCurrentWidget(connectPage);
}

void MainWindow::enableStateChanged(int state)
{
    if (state == Qt::Checked)
        wpa->setEnabled(true);
    else
        wpa->setEnabled(false);
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
