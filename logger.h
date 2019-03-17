#ifndef LOGGER_H
#define LOGGER_H

#include <vector>
#include <string>
#include <chrono>
#include <stdint.h>
#include <mutex>
#include <ctime>
#include <iostream>

#include "date.h"

class LoggerProvider;

class Logger
{
public:
    ~Logger()
    {
        std::cout << "[DTOR]\tLogger(\"" << name << "\")" << std::endl;
    }

    void erase()
    {
        std::unique_lock<std::mutex> lock(contentLocker);
        messages.clear();
    }

    void log(std::string &&msg)
    {
        std::unique_lock<std::mutex> lock(contentLocker);
        messages.emplace_back(
                    std::chrono::system_clock::now()/*std::chrono::steady_clock::now()*/,
                    std::forward<std::string>(msg));
    }

    void logError(std::string &&msg)
    {
        loggerError->log(std::string("[" + name + "] " + msg));
        log(std::forward<std::string>(msg));
    }

    void logWarning(std::string &&msg)
    {
        loggerWarning->log(std::string("[" + name + "] " + msg));
        log(std::forward<std::string>(msg));
    }

    std::string getLog()
    {
        std::unique_lock<std::mutex> lock(contentLocker);
        std::string log;
        {
            for(auto &entry : messages)
            {
                log += date::format("%F %T: ",entry.timestamp);
                log += entry.msg;
                log += "\n";
            }
        }
        return log;
    }

    bool isEnabled() const {return enabled;}

    uint32_t getOccupiedSpace()
    {
        std::unique_lock<std::mutex> lock(contentLocker);
        uint32_t size = sizeof(std::vector<msg_t>);
        size += sizeof(msg_t) * messages.size();
        for(auto &s : messages)
        {
            size += s.msg.size();
        }
        return size;
    }

    uint32_t getLogCount()
    {
        std::unique_lock<std::mutex> lock(contentLocker);
        return messages.size();
    }

private:
    Logger(
            std::string name,
            bool enabled,
            std::shared_ptr<Logger> loggerError,
            std::shared_ptr<Logger> loggerWarning) :
        name(name),
        enabled(enabled),
        loggerError(loggerError),
        loggerWarning(loggerWarning)
    {
        log(name + " log created");
    }

    struct msg_t
    {
        std::chrono::system_clock::time_point timestamp;
        std::string msg;

        msg_t(std::chrono::system_clock::time_point timestamp, std::string msg) :
            timestamp(timestamp), msg(msg)
        {}
    };

    int id;
    std::string name;
    bool enabled;

    std::shared_ptr<Logger> loggerError;
    std::shared_ptr<Logger> loggerWarning;

    std::vector<msg_t> messages;

    std::mutex contentLocker;

    friend class LoggerProvider;
};


class LoggerProvider
{
public:
    static LoggerProvider& Instance()
    {
        static LoggerProvider s;
        return s;
    }

    std::shared_ptr<Logger> createLogger(std::string name, bool enable)
    {
        std::shared_ptr<Logger> newLogger(new Logger(name, enable, logError, logWarning));
        registerLogger(newLogger);
        return newLogger;
    }

private:
    LoggerProvider()
    {
        logError = createLogger("Error", true);
        logWarning = createLogger("Warning", true);
    }

    ~LoggerProvider()
    {
        std::cout << "[DTOR]\tLoggerProvider" << std::endl;
    }

    void registerLogger(std::shared_ptr<Logger> logger)
    {
        std::unique_lock<std::mutex> lock(locker);
        registeredLoggers.push_back(logger);
    }

    std::mutex locker;

    std::shared_ptr<Logger> logError;
    std::shared_ptr<Logger> logWarning;

    std::vector<std::shared_ptr<Logger> > registeredLoggers;
};

#endif // LOGGER_H
