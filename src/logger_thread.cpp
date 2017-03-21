#include "logger_thread.h"
#include <cstdint>
#include <thread>

namespace log {

LoggerThread::LoggerThread() : m_queue(SIZE_MAX) {
    std::thread th(&LoggerThread::run, this);
    th.detach();
}

void LoggerThread::Start() {
    std::thread th(&LoggerThread::run, this);
    th.detach();
}

void LoggerThread::run() {
    while (true) {
        std::string msg;
        m_queue.Pop(&msg);
        printf("%s\n", msg.c_str());
    }
}

void LoggerThread::Send(std::string&& msg) {
    m_queue.Push(std::move(msg));
}

} // namespace log
