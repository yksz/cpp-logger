#pragma once

#include <cstddef>
#include <string>
#include "logger_queue.h"

namespace log {

class LoggerThread final {
public:
    LoggerThread();
    ~LoggerThread() = default;
    LoggerThread(const LoggerThread&) = delete;
    LoggerThread& operator=(const LoggerThread&) = delete;

    void Start();
    void Send(std::string&& msg);

private:
    LoggerQueue<std::string> m_queue;

    void run();
};

} // namespace log
