#include "mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    :QWidget(parent)
{
    setupUi(this);
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
    stackedWidget->setCurrentWidget(settingsPage);
}

void MainWindow::startGroupClicked()
{
    qDebug() << "start clicked";
}
