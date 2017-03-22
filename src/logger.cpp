#include "logger.h"
#include "logger_thread.h"

namespace log {

Logger::Logger() {
    m_thread = new LoggerThread();
}

Logger::~Logger() {
    delete m_thread;
}

void Logger::Println(std::string& msg) {
    m_thread->Send(std::string(msg));
}

void Logger::Println(std::string&& msg) {
    m_thread->Send(std::move(msg));
}

} // namespace log
