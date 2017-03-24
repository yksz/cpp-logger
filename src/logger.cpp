#include "logger.h"
#include "log_thread.h"

namespace log {

void Logger::Init() {
    auto thread = Logger::Instance().Thread();
    thread->AddWriter(std::unique_ptr<StdoutLogWriter>(new StdoutLogWriter()));
    thread->AddWriter(std::unique_ptr<StderrLogWriter>(new StderrLogWriter()));
}

Logger::Logger() : m_level(LogLevel::INFO) {
    m_thread = std::make_shared<LogThread>();
}

Logger::~Logger() {}

void Logger::Log(LogLevel level, std::string& msg) {
    if (!IsEnabled(level)) {
        return;
    }
    m_thread->Send(std::string(msg));
}

void Logger::Log(LogLevel level, std::string&& msg) {
    if (!IsEnabled(level)) {
        return;
    }
    m_thread->Send(std::move(msg));
}

inline LogLevel Logger::Level() {
    return m_level;
}

inline void Logger::SetLevel(LogLevel level) {
    m_level = level;
}

inline bool Logger::IsEnabled(LogLevel level) {
    return m_level <= level;
}

inline std::shared_ptr<LogThread> Logger::Thread() {
    return m_thread;
}

} // namespace log
