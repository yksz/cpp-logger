#pragma once

#include <memory>
#include <thread>
#include <vector>
#include "log_message.h"
#include "log_queue.h"
#include "log_writer.h"

namespace logger {

class LogThread final {
public:
    LogThread();
    ~LogThread();
    LogThread(const LogThread&) = delete;
    LogThread& operator=(const LogThread&) = delete;

    void Send(LogMessage&& msg);
    void AddWriter(std::unique_ptr<LogWriter> writer);

private:
    LogQueue<LogMessage> m_queue;
    std::thread m_thread;
    std::vector<std::unique_ptr<LogWriter>> m_writers;

    void run();
    void write(const LogMessage& msg);
};

} // namespace logger
