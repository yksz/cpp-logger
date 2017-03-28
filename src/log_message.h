#pragma once

#include <chrono>
#include <string>

namespace log {

enum struct LogLevel;

using LogClock = std::chrono::system_clock;

struct LogMessage {
    LogLevel level;
    LogClock::time_point timestamp;
    std::string content;
    bool exited;
};

} // namespace log
