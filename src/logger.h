#pragma once

#include <memory>
#include <string>

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
    void Log(LogLevel level, std::string& msg);
    void Log(LogLevel level, std::string&& msg);
    LogLevel Level();
    void SetLevel(LogLevel level);
    bool IsEnabled(LogLevel level);
    std::shared_ptr<LogThread> Thread();

private:
    LogLevel m_level;
    std::shared_ptr<LogThread> m_thread;

    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

} // namespace log
