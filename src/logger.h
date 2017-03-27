#pragma once

#include <memory>
#include <string>

#define LOG_TRACE(fmt, ...) log::Logger::Instance().Log(log::LogLevel::TRACE, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) log::Logger::Instance().Log(log::LogLevel::DEBUG, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  log::Logger::Instance().Log(log::LogLevel::INFO , fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  log::Logger::Instance().Log(log::LogLevel::WARN , fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) log::Logger::Instance().Log(log::LogLevel::ERROR, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) log::Logger::Instance().Log(log::LogLevel::FATAL, fmt, ##__VA_ARGS__)

namespace log {

enum struct LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
};

class LogThread;

class Logger final {
public:
    static Logger& Instance() {
        static Logger s_instance;
        return s_instance;
    }

    static void Init();
    LogLevel Level();
    void SetLevel(LogLevel level);
    bool IsEnabled(LogLevel level);
    std::shared_ptr<LogThread> Thread();
    void Log(LogLevel level, const char* fmt, ...);

private:
    LogLevel m_level;
    std::shared_ptr<LogThread> m_thread;

    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

} // namespace log
