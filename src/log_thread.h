#pragma once

#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "log_queue.h"
#include "log_writer.h"

namespace log {

class LogThread final {
public:
    LogThread();
    ~LogThread();
    LogThread(const LogThread&) = delete;
    LogThread& operator=(const LogThread&) = delete;

    void Send(std::string&& msg);
    void AddWriter(std::unique_ptr<LogWriter> writer);

private:
    const std::string m_poison;
    LogQueue<std::string> m_queue;
    std::thread m_thread;
    std::vector<std::unique_ptr<LogWriter>> m_writers;

    void run();
};

} // namespace log
