TEMPLATE = app

RESOURCES = wifi-direct-ui.qrc
QT += gui
DEPENDPATH += . src
INCLUDEPATH += . src

RCC_DIR = tmp
UI_DIR = tmp
MOC_DIR = tmp
OBJECTS_DIR = tmp
DESTDIR = tmp


CONFIG += debug_and_release
CONFIG(debug, debug|release) {
    TARGET = wifi-direct-demo_debug
    QMAKE_CXXFLAGS += -O0
    DEFINES += DEBUG
} else {
    TARGET = wifi-direct-demo
}

INSTALLDIR= $$(INSTALLDIR)
isEmpty(INSTALLDIR) {
    INSTALLDIR = $$(PWD)/rootfs/
}
message ("The application will be installed in $$INSTALLDIR")

target.path = $$INSTALLDIR/usr/bin
INSTALLS += target

FORMS += ui/mainwindow.ui

HEADERS += src/mainwindow.h

SOURCES += src/main.cpp         \
           src/mainwindow.cpp
