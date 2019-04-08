#pragma once

#include <cstdint>
#include <cstdio>
#include <cstring>

#if defined(_WIN32) || defined(_WIN64)
 #define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
 #define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif // defined(_WIN32) || defined(_WIN64)

#define LOG_TRACE(fmt, ...) logger::Log(logger::LogLevel_TRACE, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) logger::Log(logger::LogLevel_DEBUG, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  logger::Log(logger::LogLevel_INFO , __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  logger::Log(logger::LogLevel_WARN , __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) logger::Log(logger::LogLevel_ERROR, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) logger::Log(logger::LogLevel_FATAL, __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)

namespace logger {

enum LogLevel : uint8_t {
    LogLevel_TRACE,
    LogLevel_DEBUG,
    LogLevel_INFO,
    LogLevel_WARN,
    LogLevel_ERROR,
    LogLevel_FATAL,
};

bool InitConsoleLogger(FILE* output = stdout);
bool InitFileLogger(const char* filename, int64_t maxFileSize, uint8_t maxBackupFiles);
void SetLevel(LogLevel level);
LogLevel GetLevel();
bool IsEnabled(LogLevel level);
void Log(LogLevel level, const char* file, uint32_t line, const char* fmt, ...);

} // namespace logger
