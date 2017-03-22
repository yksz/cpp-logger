#pragma once

#include <cstddef>
#include <string>
#include <thread>
#include "log_queue.h"

namespace log {

class LogThread final {
public:
    LogThread();
    ~LogThread();
    LogThread(const LogThread&) = delete;
    LogThread& operator=(const LogThread&) = delete;

    void Send(std::string&& msg);

private:
    const std::string m_poison;
    LogQueue<std::string> m_queue;
    std::thread m_thread;

    void run();
};

} // namespace log
