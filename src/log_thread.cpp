#include "log_thread.h"
#include <cstdint>

namespace log {

LogThread::LogThread()
        : m_queue(SIZE_MAX), m_thread(&LogThread::run, this) {
}

LogThread::~LogThread() {
    m_queue.Push(std::string(m_poison));
    m_thread.join();
}

void LogThread::run() {
    while (true) {
        std::string msg;
        m_queue.Pop(&msg);
        if (msg == m_poison) {
            break;
        }
        printf("%s\n", msg.c_str());
    }
}

void LogThread::Send(std::string&& msg) {
    if (msg != m_poison) {
        m_queue.Push(std::move(msg));
    }
}

} // namespace log
