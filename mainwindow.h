#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <future>
#include "qt_gui_thread_helper.h"
#include "thread_pool.h"

#include "logger.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;

    std::shared_ptr<QtThreadsafeUICallbackProvider> provider;
    QLabel *time;

    std::thread_pool pool;

    std::shared_ptr<Logger> logger;

    void foo();
};

#endif // MAINWINDOW_H
