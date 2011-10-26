#include "mainwindow.h"

#include "keyboard.h"

#include <QDebug>
#include <QLineEdit>

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

#if !defined(DEBUG)
    QWSServer *qws = QWSServer::instance();
    if (qws)
        qws->setCursorVisible(false);
#endif

    dynamic_cast<QVBoxLayout *>(layout())->setStretchFactor(scrollArea, 1);

    connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)), this,
            SLOT(focusChanged(QWidget*, QWidget*)));
}

MainWindow::~MainWindow()
{
}

void MainWindow::backClicked()
{
    stackedWidget->setCurrentWidget(mainPage);
}

void MainWindow::channelChanged(int value)
{
    qDebug() << "channel value: " << value;
}

void MainWindow::enableStateChanged(int state)
{
    if (state == Qt::Checked)
        qDebug() << "Checked";
    else
        qDebug() << "Not Checked";
}

void MainWindow::exitClicked()
{
    close();
}

void MainWindow::focusChanged(QWidget *old, QWidget *now)
{
    Q_UNUSED(old);

    if (!now)
        return;

    if (now->inherits("QLineEdit") || now->inherits("QTextEdit")) {
        keyboard->setVisible(true);
        scrollArea->ensureWidgetVisible(now);
    } else
        keyboard->setVisible(false);
}

void MainWindow::intentChanged(int value)
{
    qDebug() << "intent value: " << value;
}

void MainWindow::refreshClicked()
{
    qDebug() << "refresh clicked";
}

void MainWindow::settingsClicked()
{
    backButton->setFocus(Qt::OtherFocusReason);
    stackedWidget->setCurrentWidget(settingsPage);
}

void MainWindow::startGroupClicked()
{
    qDebug() << "start clicked";
}
