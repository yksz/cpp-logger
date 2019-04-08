#include "logger.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#if defined(_WIN32) || defined(_WIN64)
 #include <winsock2.h>
#else
 #include <pthread.h>
 #include <sys/syscall.h>
 #include <unistd.h>
#endif // defined(_WIN32) || defined(_WIN64)
#include "log_thread.h"

namespace logger {

LogLevel s_level = LogLevel_INFO;
std::shared_ptr<LogThread> s_thread = std::make_shared<LogThread>();

bool InitConsoleLogger(FILE* output) {
    if (output == stderr) {
        s_thread->AddWriter(std::unique_ptr<StderrLogWriter>(new StderrLogWriter()));
    } else {
        s_thread->AddWriter(std::unique_ptr<StdoutLogWriter>(new StdoutLogWriter()));
    }
    return true;
}

bool InitFileLogger(const char* filename, int64_t maxFileSize, uint8_t maxBackupFiles) {
    auto writer = std::unique_ptr<FileLogWriter>(new FileLogWriter(filename, maxFileSize, maxBackupFiles));
    if (!writer->Init()) {
        return false;
    }
    s_thread->AddWriter(std::move(writer));
    return true;
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

static int gettimeofday(struct timeval* tv, void* tz) {
    const UINT64 epochFileTime = 116444736000000000ULL;

    if (tv == NULL) {
        return -1;
    }
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER li;
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    UINT64 t = (li.QuadPart - epochFileTime) / 10;
    tv->tv_sec = (long) (t / 1000000);
    tv->tv_usec = t % 1000000;
    return 0;
}
#endif // defined(_WIN32) || defined(_WIN64)

static uint64_t getCurrentThreadID();

void Log(LogLevel level, const char* file, uint32_t line, const char* fmt, ...) {
    if (!IsEnabled(level)) {
        return;
    }

    char* buf = nullptr;
    va_list arg;
    va_start(arg, fmt);
    if (vasprintf(&buf, fmt, arg) != -1) {
        LogMessage msg = {};
        msg.level = level;
        gettimeofday(&msg.timestamp, nullptr);
        msg.threadID = getCurrentThreadID();
        msg.file = file;
        msg.line = line;
        msg.content = std::unique_ptr<char>(buf);
        s_thread->Send(std::move(msg));
    } else {
        fprintf(stderr, "ERROR: logger: vasprintf");
    }
    va_end(arg);
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

void SetLevel(LogLevel level) {
    s_level = level;
}

LogLevel GetLevel() {
    return s_level;
}

bool IsEnabled(LogLevel level) {
    return s_level <= level;
}

} // namespace logger
