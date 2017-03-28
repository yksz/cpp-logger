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
    if (msg.exited) {
        return;
    }
    m_queue.Push(std::move(msg));
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
    }
}

static char toCharacter(const LogLevel& level);
static std::string toString(const LogClock::time_point& timestamp);

static std::string format(const LogMessage& msg) {
    std::stringstream ss;
    ss << toCharacter(msg.level)
       << " "
       << toString(msg.timestamp)
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
static struct tm* localtime_r(const time_t* timep, struct tm* result)
{
    localtime_s(result, timep);
    return result;
}
#endif // defined(_WIN32) || defined(_WIN64)

template<class Duration>
static Duration toDuration(const LogClock::time_point& tp) {
    auto count = std::chrono::time_point_cast<Duration>(tp).time_since_epoch().count();
    return Duration(count);
}

static std::string toString(const LogClock::time_point& timestamp) {
    using namespace std::chrono;

    time_t time = LogClock::to_time_t(timestamp);
    struct tm calendar;
    localtime_r(&time, &calendar);
    char timestr[32];
    strftime(timestr, sizeof(timestr), "%y-%m-%d %H:%M:%S", &calendar);

    microseconds duration = toDuration<microseconds>(timestamp);
    long usec = duration.count() % 1000000;

    std::stringstream ss;
    ss << timestr
       << "."
       << std::setfill('0') << std::setw(6) << usec; // %06ld
    return ss.str();
}

} // namespace log
