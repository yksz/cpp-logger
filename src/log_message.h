#pragma once

#include <cstdint>
#include <memory>
#if defined(_WIN32) || defined(_WIN64)
 #include <winsock2.h>
#else
 #include <sys/time.h>
#endif // defined(_WIN32) || defined(_WIN64)

namespace logger {

enum LogLevel : uint8_t;

struct LogMessage {
    LogLevel level;
    struct timeval timestamp;
    uint64_t threadID;
    const char* file;
    uint32_t line;
    std::unique_ptr<char> content;
    bool exited;
};

} // namespace logger
