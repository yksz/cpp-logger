#include "log_thread.h"
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "logger.h"

namespace logger {

LogThread::LogThread()
        : m_queue(SIZE_MAX), m_thread(&LogThread::run, this) {}

LogThread::~LogThread() {
    LogMessage exit = {};
    exit.exited = true;
    m_queue.Push(std::move(exit));
    m_thread.join();
}

void LogThread::Send(LogMessage&& msg) {
    if (!msg.exited) {
        m_queue.Push(std::move(msg));
    }
}

void LogThread::AddWriter(std::unique_ptr<LogWriter> writer) {
    m_writers.push_back(std::move(writer));
}

void LogThread::run() {
    while (true) {
        LogMessage msg;
        m_queue.Pop(&msg);
        if (msg.exited) {
            break;
        }
        write(msg);
        free(msg.content);
    }
}

static char toCharacter(LogLevel level);
static void toString(const struct timeval& time, char* str, size_t len);

void LogThread::write(const LogMessage& msg) {
    char level = toCharacter(msg.level);
    char timestamp[32];
    toString(msg.timestamp, timestamp, sizeof(timestamp));
    for (auto& writer : m_writers) {
        writer->Print(level, timestamp, msg);
    }
}

static char toCharacter(LogLevel level) {
    switch (level) {
        case LogLevel_TRACE: return 'T';
        case LogLevel_DEBUG: return 'D';
        case LogLevel_INFO:  return 'I';
        case LogLevel_WARN:  return 'W';
        case LogLevel_ERROR: return 'E';
        case LogLevel_FATAL: return 'F';
        default: return ' ';
    }
}

#if defined(_WIN32) || defined(_WIN64)
static struct tm* localtime_r(const time_t* timep, struct tm* result) {
    localtime_s(result, timep);
    return result;
}
#endif // defined(_WIN32) || defined(_WIN64)

static void toString(const struct timeval& time, char* str, size_t len) {
    time_t sec = time.tv_sec;
    struct tm calendar;
    localtime_r(&sec, &calendar);
    strftime(str, len, "%y-%m-%d %H:%M:%S", &calendar);
    const int offset = 17;
    if (len > offset) {
        snprintf(&str[offset], len - offset, ".%06ld", (long) time.tv_usec);
    }
}

} // namespace logger
