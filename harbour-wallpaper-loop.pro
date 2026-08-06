TARGET = harbour-wallpaper-loop

CONFIG += sailfishapp c++14

QT += core gui qml quick dbus

SOURCES += \
    src/harbour-wallpaper-loop.cpp \
    src/loopcontroller.cpp

HEADERS += \
    src/loopcontroller.h

DISTFILES += \
    qml/harbour-wallpaper-loop.qml \
    qml/cover/CoverPage.qml \
    qml/pages/MainPage.qml \
    qml/pages/FolderPage.qml \
    qml/pages/AboutPage.qml \
    rpm/harbour-wallpaper-loop.changes \
    rpm/harbour-wallpaper-loop.changes.in \
    rpm/harbour-wallpaper-loop.profile \
    rpm/harbour-wallpaper-loop.spec \
    harbour-wallpaper-loop.desktop \
    icons/harbour-wallpaper-loop.svg \
    LICENSE \
    README.md \
    openrepos/listing.md

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172
