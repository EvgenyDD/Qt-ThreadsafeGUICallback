#-------------------------------------------------
#
# Project created by QtCreator 2019-03-13T07:39:18
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11
CONFIG += c++11


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h \
    qt_gui_thread_helper.h \
    thread_pool.h \
    logger.h \
    date.h \
    queue_threadsafe.h

FORMS    += mainwindow.ui

LIBS = -lpsapi
