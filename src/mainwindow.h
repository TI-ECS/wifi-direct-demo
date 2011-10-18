#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include "ui_mainwindow.h"

class MainWindow : public QWidget, public Ui::MainWindow
{
Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();
};


#endif /* _MAINWINDOW_H_ */
