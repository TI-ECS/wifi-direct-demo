#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include "ui_mainwindow.h"

class MainWindow : public QWidget, public Ui::MainWindow
{
Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

public slots:
    void backClicked();
    void channelChanged(int value);
    void enableStateChanged(int state);
    void exitClicked();
    void intentChanged(int value);
    void refreshClicked();
    void settingsClicked();
    void startGroupClicked();
};


#endif /* _MAINWINDOW_H_ */
