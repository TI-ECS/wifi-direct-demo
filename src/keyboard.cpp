#include "keyboard.h"

#include <QApplication>


Keyboard::Keyboard(QWidget *parent)
    :QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(3);
    line1 = new QHBoxLayout;
    line2 = new QHBoxLayout;
    line3 = new QHBoxLayout;
    line4 = new QHBoxLayout;

    mainLayout->addLayout(line1, 0);
    mainLayout->addLayout(line2, 0);
    mainLayout->addLayout(line3, 0);
    mainLayout->addLayout(line4, 0);

    setLayout(mainLayout);

    createButtons();
}

Keyboard::~Keyboard()
{
    while (!buttons.isEmpty())
        buttons.removeFirst();
}

void Keyboard::createButtons()
{
    QStringList lineItems;
    QPushButton *b;

    lineItems  << "1" << "2" << "3" << "4" << "5" << "6"
               << "7" << "8"<< "9" << "0";
    line1->insertStretch(0, 4);
    foreach (const QString &label, lineItems) {
        b = new QPushButton(label);
        b->setMaximumSize(QSize(35,30));
        buttons.append(b);
        line1->addWidget(b);
    }
    b = new QPushButton("Backspace");
    buttons.append(b);
    b->setMaximumHeight(30);
    line1->addWidget(b);
    line1->insertStretch(-1, 4);

    lineItems.clear();
    lineItems << "q" << "w" << "e" << "r" << "t" << "y" << "u"
               << "i" << "o"<< "p";
    line2->insertStretch(0, 4);
    foreach (const QString &label, lineItems) {
        b = new QPushButton(label);
        b->setMaximumSize(QSize(35,30));
        buttons.append(b);
        line2->addWidget(b);
    }
    line2->insertStretch(-1, 4);

    lineItems.clear();
    lineItems << "a" << "s" << "d" << "f" << "g" << "h" << "j"
               << "k" << "l";
    line3->insertStretch(0, 4);
    foreach (const QString &label, lineItems) {
        b = new QPushButton(label);
        b->setMaximumSize(QSize(35,30));
        buttons.append(b);
        line3->addWidget(b);
    }
    line3->insertStretch(-1, 4);

    lineItems.clear();
    lineItems << "z" << "x" << "c" << "v" << "b" << "n" << "m";
    line4->insertStretch(0, 4);
    foreach (const QString &label, lineItems) {
        b = new QPushButton(label);
        buttons.append(b);
        b->setMaximumSize(QSize(35,30));
        line4->addWidget(b);
    }
    line4->insertStretch(-1, 4);

    foreach (b, buttons) {
        b->setFocusPolicy(Qt::NoFocus);
        connect(b, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    }
}

void Keyboard::buttonClicked()
{
    QPushButton *button  = dynamic_cast<QPushButton *>(sender());
    if (!button)
        return;

    QString text = button->text();
    if (text == "Backspace") {
        QApplication::sendEvent(qApp->focusWidget(),
                                new QKeyEvent(QEvent::KeyPress, Qt::Key_Backspace,
                                              Qt::NoModifier));
        return;
    }

    ushort keyId = button->text().data()->unicode();
    QApplication::sendEvent(qApp->focusWidget(),
                            new QKeyEvent(QEvent::KeyPress, keyId,
                                          Qt::NoModifier,
                                          text));
}
