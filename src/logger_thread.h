#pragma once

#include <cstddef>
#include <string>
#include <thread>
#include "logger_queue.h"

namespace log {

class LoggerThread final {
public:
    LoggerThread();
    ~LoggerThread();
    LoggerThread(const LoggerThread&) = delete;
    LoggerThread& operator=(const LoggerThread&) = delete;

    void Send(std::string&& msg);

private:
    const std::string m_poison;
    LoggerQueue<std::string> m_queue;
    std::thread m_thread;

    void run();
};

} // namespace log
