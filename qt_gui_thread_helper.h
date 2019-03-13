#ifndef GUI_THREAD_HELPER_H
#define GUI_THREAD_HELPER_H

#include <QObject>
#include <thread_helper.h>

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

    void pushAction(std::function<void()> &&action)
    {
        actions.push(std::forward<decltype(action)>(action));
        emit actionAppended();
    }

signals:
    void actionAppended();

private:
    std::threadsafe_queue<std::function<void()>> actions;
};

#endif // GUI_THREAD_HELPER_H
