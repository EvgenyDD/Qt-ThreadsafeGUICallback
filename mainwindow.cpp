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


#include<windows.h>
#include<stdio.h>
#include<tchar.h>
#include "psapi.h"


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
        static uint32_t staticIterator=1;
        temp.number = staticIterator++;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    return temp;
}
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    pool(100)
{
    ui->setupUi(this);

    provider = std::shared_ptr<QtThreadsafeUICallbackProvider>(new QtThreadsafeUICallbackProvider());
    logger = LoggerProvider::Instance().createLogger("UI", true);

    time = new QLabel("Time");
    ui->statusBar->addWidget(time);


    auto timer = new QTimer();
    timer->setInterval(20);
    connect(timer, &QTimer::timeout, [&](){time->setText(QTime::currentTime().toString("<b>hh:mm:ss|zzz </b>"));});
    timer->start();

    auto timer2 = new QTimer();
    timer2->setInterval(2500);
    connect(timer2, &QTimer::timeout, [&](){
        PROCESS_MEMORY_COUNTERS_EX pmc;
        GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(PROCESS_MEMORY_COUNTERS_EX));
        SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;

        qDebug() << (float)(virtualMemUsedByMe/1024)*(1./1024.) << "MB" << pool.getJobCount();
        foo();
    });
    timer2->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::foo()
{
    for(uint32_t i=0; i<50; i++)
    {
        pool.addJob([&](){
            auto value = RESOURCE::getResult();
            std::shared_ptr<std::vector<int> > d(new std::vector<int>);
            d->resize(500*1024);
            (*d)[0]++;
            provider->pushAction(

            /* async GUI thread compatible callback */
            [&, value, d = std::move(d)]() {
                static int iii = 1;
                (*d)[0]=1;
                logger->log("Threadwork");
                ui->pushButton->setText(QString::number(value.number) + "-" + QString::number(iii));
                iii++;
            });
        });
    }
}

void MainWindow::on_pushButton_clicked()
{
    foo();

    std::string str = logger->getLog();
    std::cout << logger->getOccupiedSpace() << " " << logger->getLogCount() << " " << str << std::endl;
}
