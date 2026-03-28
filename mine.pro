QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mine
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/minefield.cpp \
    src/minewindow.cpp

HEADERS += \
    src/minefield.h \
    src/minewindow.h

FORMS += \
    ui/minewindow.ui

RESOURCES += \
    resources/resources.qrc