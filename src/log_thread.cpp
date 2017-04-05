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

static std::string format(const LogMessage& msg);

void LogThread::run() {
    while (true) {
        LogMessage msg;
        m_queue.Pop(&msg);
        if (msg.exited) {
            break;
        }
        std::string formatted = format(msg);
        for (auto& writer : m_writers) {
            writer->Print(formatted);
        }
        free(msg.content);
    }
}

static char toCharacter(const LogLevel& level);
static void toString(const struct timeval& timestamp, char* timestr, size_t len);

static std::string format(const LogMessage& msg) {
    char timestr[32];
    toString(msg.timestamp, timestr, sizeof(timestr));

    std::stringstream ss;
    ss << toCharacter(msg.level)
       << " "
       << timestr
       << " "
       << msg.threadID
       << " "
       << msg.file
       << ":"
       << msg.line
       << ": "
       << msg.content
       << std::endl;
    return ss.str();
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

static void toString(const struct timeval& timestamp, char* timestr, size_t len) {
    struct tm calendar;
    localtime_r(&timestamp.tv_sec, &calendar);
    strftime(timestr, len, "%y-%m-%d %H:%M:%S", &calendar);
    sprintf(&timestr[17], ".%06ld", (long) timestamp.tv_usec);
}

} // namespace log
