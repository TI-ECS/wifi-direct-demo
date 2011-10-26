#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QList>
#include <QPushButton>
#include <QWidget>


class Keyboard : public QWidget
{

Q_OBJECT

public:
    Keyboard(QWidget *parent = 0);
    virtual ~Keyboard();

private slots:
    void buttonClicked();

private:
    void createButtons();

    QHBoxLayout *line1, *line2, *line3, *line4;
    QList<QPushButton *> buttons;
};


#endif /* _KEYBOARD_H_ */
