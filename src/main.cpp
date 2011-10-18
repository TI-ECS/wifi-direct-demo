#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.setWindowTitle(QObject::tr("Wifi-Direct demo"));

#if defined(DEBUG)
    window.setMaximumSize(800, 480);
    window.show();
#else
    window.showFullScreen();
#endif

    return app.exec();
}
