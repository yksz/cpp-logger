#include "logger_thread.h"
#include <cstdint>

namespace log {

LoggerThread::LoggerThread()
        : m_queue(SIZE_MAX), m_thread(&LoggerThread::run, this) {
}

LoggerThread::~LoggerThread() {
    m_queue.Push(std::string(m_poison));
    m_thread.join();
}

void LoggerThread::run() {
    while (true) {
        std::string msg;
        m_queue.Pop(&msg);
        if (msg == m_poison) {
            break;
        }
        printf("%s\n", msg.c_str());
    }
}

void LoggerThread::Send(std::string&& msg) {
    if (msg != m_poison) {
        m_queue.Push(std::move(msg));
    }
}

} // namespace log
