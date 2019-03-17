#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <thread>
#include <vector>
#include <list>
#include <memory>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <tuple>
#include <iostream>
#include <deque>

namespace std
{

/**
 * @brief The ThreadPool class launches a pre-defined number of threads and
 *        keeps them ready to execute jobs.
 *
 * Each job is composed by two functions (std::function<void()>):
 * the two functions are executed one after another, and the second one
 *  usually sends some sort of notification that the job has been done.
 *
 */
class thread_pool
{
public:

    /**
     * @brief Initializes the pool and launches the threads.
     *
     * @param numThreads number of threads to launch.
     */
    thread_pool(size_t numThreads):
        workers(numThreads),
        terminate(false)
    {
        for(std::unique_ptr<std::thread>& worker: workers)
        {
            worker.reset(new std::thread(&thread_pool::loop, this));
        }
    }


    /**
     * @brief Destructors
     *
     * Sends a "terminate" signal to the threads and waits for
     *  their termination.
     *
     * The threads will complete the currently running job
     *  before checking for the "terminate" flag.
     */
    virtual ~thread_pool()
    {
        std::cout << "[DTOR]\tthread_pool" << std::endl;
        {
            std::unique_lock<std::mutex> lockList(lockJobsList);
            terminate = true;
            notifyJob.notify_all();
        }

        for(std::unique_ptr<std::thread>& worker: workers)
        {
            worker->join();
        }
    }


    /**
     * @brief Schedule a job for execution.
     *
     * The first available thread will pick up the job and run it.
     *
     * @param job             a function that executes the job. It is called
     *                         in the thread that pickec it up
     * @param notificationJob a function that usually sends a notification
     *                         that the job has been executed. it is executed
     *                         immediately after the first function in the same
     *                         thread that ran the first function
     */
    void addJob(std::function<void()> &&job)
    {
        std::unique_lock<std::mutex> lockList(lockJobsList);
        jobs.push_back(std::move(job));
        notifyJob.notify_one();
    }


    size_t getJobCount()
    {
        std::unique_lock<std::mutex> lockList(lockJobsList);
        auto size = jobs.size();
        return size;
    }

private:
    /**
     * @brief Function executed in each worker thread.
     *
     * Runs until the termination flag terminate is set to true
     *  in the class destructor
     */
    void loop()
    {
        try
        {
            for(;;)
            {
                std::function<void()> job = getNextJob();
                job();
            }
        }
        catch(Terminated& e)
        {
            std::cout << "[ERROR]\t" << __FUNCTION__ << ": " << e.what() <<  std::endl;
        }
    }


    /**
     * @brief Returns the next job scheduled for execution.
     *
     * The function blocks if the list of scheduled jobs is empty until
     *  a new job is scheduled or until m_bTerminate is set to true
     *  by the class destructor, in which case it throws Terminated.
     *
     * When a valid job is found it is removed from the queue and
     *  returned to the caller.
     *
     * @return the next job to execute
     */
    std::function<void()>  getNextJob()
    {
        std::unique_lock<std::mutex> lockList(lockJobsList);

        while(!terminate)
        {
            if(!jobs.empty())
            {
                std::function<void()> job = jobs.front();
                jobs.pop_front();
                return job;
            }

            notifyJob.wait(lockList);
        }

        throw Terminated("Thread terminated");
    }


    /**
     * @brief Contains the running working threads (workers).
     */
    std::vector<std::unique_ptr<std::thread> > workers;


    /**
     * @brief Queue of jobs scheduled for execution and not yet executed.
     */
    std::deque<std::function<void()> > jobs;


    /**
     * @brief Mutex used to access the queue of scheduled jobs (jobs).
     */
    std::mutex lockJobsList;


    /**
     * @brief Condition variable used to notify that a new job has been
     *        inserted in the queue (jobs).
     */
    std::condition_variable notifyJob;


    /**
     * @brief This flag is set to true by the class destructor to signal
     *         the worker threads that they have to terminate.
     */
    std::atomic<bool> terminate;


    /**
     * @brief This exception is thrown by getNextJob() when the flag
     *         terminate has been set.
     */
    class Terminated: public std::runtime_error
    {
    public:
        Terminated(const std::string& what): std::runtime_error(what) {}
    };

};
}

#endif // THREAD_POOL_H
