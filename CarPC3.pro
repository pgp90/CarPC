#-------------------------------------------------
#
# Project created by QtCreator 2014-01-23T13:06:41
#
#-------------------------------------------------

QT += core gui
QT += multimedia
QT += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CarPC3
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    obd2widget.cpp \
    playerinfowidget.cpp \
    volumecontrolwidget.cpp
#SOURCES += mainwindow2.cpp

HEADERS  += mainwindow.h \
    obd2widget.h \
    playerinfowidget.h \
    volumecontrolwidget.h
#HEADERS  += mainwindow2.h

FORMS    += mainwindow.ui \
    obd2widget.ui \
    playerinfowidget.ui \
    volumecontrolwidget.ui

RESOURCES += stylesheet.qrc
