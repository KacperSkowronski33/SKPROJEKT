QT       += core gui
QT       += printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PROJEKTPK
TEMPLATE = app
CONFIG += c++17

INCLUDEPATH += \
$$PWD/backend \
$$PWD/service \
$$PWD/ui \
$$PWD/tests

SOURCES += \
    backend/ARX.cpp \
    backend/GWZ.cpp \
    backend/PID.cpp \
    tests/TestyJednostkowe.cpp \
    backend/UAR.cpp \
    service/WarstwaU.cpp \
    ui/qcustomplot.cpp \
    main.cpp \
    ui/mainwindow.cpp \
    ui/ustawieniaarx.cpp

HEADERS += \
    backend/ARX.h \
    backend/GWZ.h \
    backend/PID.h \
    tests/TestyJednostkowe.h \
    backend/UAR.h \
    service/WarstwaU.h \
    ui/qcustomplot.h \
    ui/mainwindow.h \
    ui/ustawieniaarx.h

FORMS += \
    ui/mainwindow.ui \
    ui/ustawieniaarx.ui
