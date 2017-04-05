#pragma once

#include <chrono>
#include <string>

namespace log {

enum struct LogLevel;

using LogClock = std::chrono::system_clock;

struct LogMessage {
    LogLevel level;
    LogClock::time_point timestamp;
    uint64_t threadID;
    const char* file;
    uint32_t line;
    char* content;
    bool exited;
};

} // namespace log
