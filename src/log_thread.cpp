#include "log_thread.h"
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "logger.h"

namespace log {

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

static char toCharacter(const LogLevel& level);
static void toString(const struct timeval& time, char* str, size_t len);

void LogThread::write(const LogMessage& msg) {
    char level = toCharacter(msg.level);
    char timestamp[32];
    toString(msg.timestamp, timestamp, sizeof(timestamp));
    for (auto& writer : m_writers) {
        writer->Print(level, timestamp, msg);
    }
}

static char toCharacter(const LogLevel& level) {
    switch (level) {
        case LogLevel::TRACE: return 'T';
        case LogLevel::DEBUG: return 'D';
        case LogLevel::INFO:  return 'I';
        case LogLevel::WARN:  return 'W';
        case LogLevel::ERROR: return 'E';
        case LogLevel::FATAL: return 'F';
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
    struct tm calendar;
    localtime_r(&time.tv_sec, &calendar);
    strftime(str, len, "%y-%m-%d %H:%M:%S", &calendar);
    const int offset = 17;
    if (len > offset) {
        snprintf(&str[offset], len - offset, ".%06ld", (long) time.tv_usec);
    }
}

} // namespace log
