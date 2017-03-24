#pragma once

#include <memory>
#include <string>

namespace log {

class LogThread;

class Logger final {
public:
    static Logger& Instance() {
        static Logger s_instance;
        return s_instance;
    }

    static void Init();

    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void Log(std::string& msg);
    void Log(std::string&& msg);

    std::shared_ptr<LogThread> Thread() {
        return m_thread;
    }

private:
    std::shared_ptr<LogThread> m_thread;
};

} // namespace log
