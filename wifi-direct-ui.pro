TEMPLATE = app

RESOURCES = wifi-direct-ui.qrc
QT += gui dbus
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

images.path = $$INSTALLDIR/usr/share/matrix/images
images.files = images/wifi-direct-icon.png

scripts.path = $$INSTALLDIR/usr/bin
scripts.files = scripts/*

INSTALLS += scripts target

FORMS += ui/mainwindow.ui

HEADERS += src/device.h             \
	   src/devicelistdelegate.h \
	   src/deviceslistmodel.h   \
           src/group.h              \
           src/interface.h          \
           src/interfaces.h         \
           src/mainwindow.h         \
           src/keyboard.h           \
           src/p2pdevice.h          \
           src/peer.h               \
           src/types.h              \
           src/wpa.h                \
           src/wps.h


SOURCES += src/device.cpp             \
	   src/devicelistdelegate.cpp \
	   src/deviceslistmodel.cpp   \
           src/group.cpp              \
           src/interface.cpp          \
           src/interfaces.cpp         \
           src/main.cpp               \
           src/mainwindow.cpp         \
           src/keyboard.cpp           \
           src/p2pdevice.cpp          \
           src/peer.cpp               \
           src/wpa.cpp                \
           src/wps.cpp
