#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QTimer"
#include "QTime"
#include <thread>
#include <future>
#include <QDebug>
#include <string>
#include <QApplication>
#include <QAbstractEventDispatcher>
#include <QThread>
#include <functional>
#include <iostream>

#include "qt_gui_thread_helper.h"

namespace RESOURCE
{
class TestClass
{
public:
    int number = 0;
    //    E(){qDebug() << "E()";}
    //    E(E&& a){qDebug() << "E(E&& a)";}
    //    E(const E& a){qDebug() << "E(const E& a)";}
    //    E &operator=(E const &) {qDebug() << "E &operator=(E const &)";}
    //    E &operator=(E &&){qDebug() << "E &operator=(E &&)";}
};

TestClass getResult()
{
    static std::mutex s;
    TestClass temp;
    {
        std::lock_guard<std::mutex> lock(s);
        static uint32_t staticIterator=0;
        temp.number = staticIterator++;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    return temp;
}
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    provider = new QtThreadsafeUICallbackProvider();

    time = new QLabel("Time");
    ui->statusBar->addWidget(time);

    auto timer = new QTimer();
    timer->setInterval(20);
    connect(timer, &QTimer::timeout, [&](){time->setText(QTime::currentTime().toString("<b>hh:mm:ss|zzz </b>"));});
    timer->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    for(uint32_t i=0; i<1024; i++)
    {
        try
        {
            std::thread([&](){
                auto value = RESOURCE::getResult();
                provider->pushAction(
                            /* async GUI thread compatible callback */
                            [&, value]() { ui->pushButton->setText(QString::number(value.number)); }
                );
            }).detach();
        }
        catch(const std::exception& exception)
        {
            qDebug() << "Exception throwed at iterator:" << i << " => " << exception.what();
            break;
        }
    }

    //    qDebug() << ">>> sleep";
    //    QThread::msleep(5000);
    //    qDebug() << "<<< sleep";
}
