#pragma once

#include <cstdio>
#include <string>

namespace log {

class LogThread;

class Logger final {
public:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void Println(std::string& msg);
    void Println(std::string&& msg);

private:
    LogThread* m_thread;
};

} // namespace log
