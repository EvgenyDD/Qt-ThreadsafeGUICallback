#ifndef THREAD_HELPER_H
#define THREAD_HELPER_H

#include <condition_variable>
#include <queue>
#include <memory>
#include <iostream>

namespace std
{

template<typename T>
class queue_threadsafe
{
private:
    mutable std::mutex mut;
    std::queue<T> data_queue;
    std::condition_variable data_cond;

public:
    queue_threadsafe(){}

        ~queue_threadsafe(){ std::cout << "[DTOR]\tqueue_threadsafe" << std::endl;}

    queue_threadsafe(queue_threadsafe const& other)
    {
        std::lock_guard<std::mutex> lock(other.mut);
        data_queue = other.data_queue;
    }

    void push(T new_value)
    {
        std::lock_guard<std::mutex> lock(mut);
        data_queue.push(new_value);
        data_cond.notify_one();
    }

    void wait_and_pop(T& value)
    {
        std::unique_lock<std::mutex> lock(mut);
        data_cond.wait(lock, [this]{return !data_queue.empty();});
        value = data_queue.front();
        data_queue.pop();
    }

    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock<std::mutex> lock(mut);
        data_cond.wait(lock, [this]{return !data_queue.empty();});
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }

    bool try_pop(T& value)
    {
        std::lock_guard<std::mutex> lock(mut);
        if(data_queue.empty()) {return false;}
        value = data_queue.front();
        data_queue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop()
    {
        std::lock_guard<std::mutex> lock(mut);
        if(data_queue.empty()) {return std::shared_ptr<T>();}
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(mut);
        return data_queue.empty();
    }
};
}

#endif // THREAD_HELPER_H
