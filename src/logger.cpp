#include "logger.h"
#include <cstdarg>
#include <cstdio>
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

void Logger::Log(LogLevel level, const char* fmt, ...) {
    if (!IsEnabled(level)) {
        return;
    }

    char* buf = nullptr;
    va_list arg;
    va_start(arg, fmt);
    bool ok = vasprintf(&buf, fmt, arg) != -1;
    if (ok) {
        m_thread->Send(buf);
    } else {
        fprintf(stderr, "ERROR: logger: vasprintf");
    }
    va_end(arg);
    if (buf != nullptr) {
        free(buf);
    }
}

} // namespace log
