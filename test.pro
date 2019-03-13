#-------------------------------------------------
#
# Project created by QtCreator 2019-03-13T07:39:18
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++1z
CONFIG += c++1z


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h \
    thread_helper.h \
    qt_gui_thread_helper.h

FORMS    += mainwindow.ui
