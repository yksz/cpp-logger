#include "logger.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#if defined(_WIN32) || defined(_WIN64)
 #include <windows.h>
#else
 #include <pthread.h>
 #include <sys/syscall.h>
 #include <unistd.h>
#endif // defined(_WIN32) || defined(_WIN64)
#include "log_thread.h"

namespace log {

void Logger::Init() {
    auto thread = Logger::Instance().Thread();
    thread->AddWriter(std::unique_ptr<StdoutLogWriter>(new StdoutLogWriter()));
    thread->AddWriter(std::unique_ptr<StderrLogWriter>(new StderrLogWriter()));
}

Logger::Logger() : m_level(LogLevel::INFO) {
    m_thread = std::make_shared<LogThread>();
}

Logger::~Logger() {}

inline LogLevel Logger::Level() {
    return m_level;
}

inline void Logger::SetLevel(LogLevel level) {
    m_level = level;
}

inline bool Logger::IsEnabled(LogLevel level) {
    return m_level <= level;
}

inline std::shared_ptr<LogThread> Logger::Thread() {
    return m_thread;
}

#if defined(_WIN32) || defined(_WIN64)
static int vasprintf(char** strp, const char* fmt, va_list ap) {
    int len = _vscprintf(fmt, ap);
    if (len == -1) {
        return -1;
    }
    size_t size = (size_t) (len + 1);
    *strp = (char*) malloc(size);
    if (*strp == nullptr) {
        return -1;
    }
    int result = vsnprintf(*strp, size, fmt, ap);
    if (result == -1) {
        free(*strp);
        return -1;
    }
    return result;
}
#endif // defined(_WIN32) || defined(_WIN64)

static uint64_t getCurrentThreadID();

void Logger::Log(LogLevel level, const char* file, uint32_t line, const char* fmt, ...) {
    if (!IsEnabled(level)) {
        return;
    }

    char* buf = nullptr;
    va_list arg;
    va_start(arg, fmt);
    if (vasprintf(&buf, fmt, arg) != -1) {
        LogMessage msg = {};
        msg.level = level;
        msg.timestamp = LogClock::now();
        msg.threadID = getCurrentThreadID();
        msg.file = file;
        msg.line = line;
        msg.content = buf;
        m_thread->Send(std::move(msg));
    } else {
        fprintf(stderr, "ERROR: logger: vasprintf");
    }
    va_end(arg);
    if (buf != nullptr) {
        free(buf);
    }
}

static uint64_t getCurrentThreadID() {
#if defined(_WIN32) || defined(_WIN64)
    return (uint64_t) GetCurrentThreadId();
#elif __linux__
    return (uint64_t) syscall(SYS_gettid);
#elif defined(__APPLE__) && defined(__MACH__)
    return (uint64_t) syscall(SYS_thread_selfid);
#else
    return (uint64_t) pthread_self();
#endif // defined(_WIN32) || defined(_WIN64)
}

} // namespace log
