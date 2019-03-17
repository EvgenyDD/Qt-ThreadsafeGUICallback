#ifndef GUI_THREAD_HELPER_H
#define GUI_THREAD_HELPER_H

#include <QObject>
#include <queue_threadsafe.h>
#include <iostream>

class QtThreadsafeUICallbackProvider : public QObject
{
    Q_OBJECT

public:
    QtThreadsafeUICallbackProvider()
    {
        /* as constructor is called from the gui thread -> its slot will exec from gui thread */
        connect(this, &QtThreadsafeUICallbackProvider::actionAppended, this, [&]()
        {
            std::function<void()> action;
            while(actions.try_pop(action))
            {
                action();
            }
        },
        Qt::QueuedConnection);
    }

    ~QtThreadsafeUICallbackProvider(){std::cout << "[DTOR]\tQtThreadsafeUICallbackProvider" << std::endl;}

    void pushAction(std::function<void()> &&action)
    {
        actions.push(std::forward<decltype(action)>(action));
        emit actionAppended();
    }

signals:
    void actionAppended();

private:
    std::queue_threadsafe<std::function<void()>> actions;
};

#endif // GUI_THREAD_HELPER_H
