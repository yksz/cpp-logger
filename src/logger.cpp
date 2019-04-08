#include "logger.h"
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <condition_variable>
#include <fstream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
#if defined(_WIN32) || defined(_WIN64)
 #include <winsock2.h>
#else
 #include <sys/syscall.h>
 #include <sys/time.h>
 #include <unistd.h>
#endif // defined(_WIN32) || defined(_WIN64)

namespace {

using namespace logger;

const int64_t kDefaultMaxFileSize = 1048576L; // 1 MB
const size_t kQueueCapacity = 1000;

#if defined(_WIN32) || defined(_WIN64)
static int vasprintf(char** strp, const char* fmt, va_list ap) {
    int len = _vscprintf(fmt, ap);
    if (len == -1) {
        return -1;
    }
    size_t size = (size_t)(len + 1);
    *strp = (char*)malloc(size);
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

    if (tv == nullptr) {
        return -1;
    }
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    ULARGE_INTEGER li;
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    UINT64 t = (li.QuadPart - epochFileTime) / 10;
    tv->tv_sec = (long)(t / 1000000);
    tv->tv_usec = t % 1000000;
    return 0;
}

static struct tm* localtime_r(const time_t* timep, struct tm* result) {
    localtime_s(result, timep);
    return result;
}
#endif // defined(_WIN32) || defined(_WIN64)

struct LogMessage {
    LogLevel level;
    struct timeval timestamp;
    uint64_t threadID;
    const char* file;
    uint32_t line;
    std::unique_ptr<char> content;
    bool exited;
};

template<typename T>
class LogQueue final {
public:
    explicit LogQueue(size_t capacity) : m_capacity(capacity) {}
    ~LogQueue() = default;
    LogQueue(const LogQueue&) = delete;
    LogQueue& operator=(const LogQueue&) = delete;

    void Push(T&& element) {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (isFull()) {
            m_notfull.wait(lock);
        }
        bool wasEmpty = isEmpty();
        m_queue.push(std::move(element));
        if (wasEmpty) {
            m_notempty.notify_one();
        }
    }

    void Pop(T* element) {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (isEmpty()) {
            m_notempty.wait(lock);
        }
        bool wasFull = isFull();
        if (element != nullptr) {
            *element = std::move(m_queue.front());
        }
        m_queue.pop();
        if (wasFull) {
            m_notfull.notify_one();
        }
    }

private:
    const size_t m_capacity;
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_notfull;
    std::condition_variable m_notempty;

    bool isEmpty() {
        return m_queue.empty();
    }

    bool isFull() {
        return m_queue.size() + 1 > m_capacity;
    }
};

struct LogWriter {
    virtual ~LogWriter() {}
    virtual void Print(char level, const char* timestamp, const LogMessage& msg) = 0;
};

class LogThread final {
public:
    LogThread() : m_queue(kQueueCapacity), m_thread(&LogThread::run, this) {}

    ~LogThread() {
        LogMessage exit = {};
        exit.exited = true;
        m_queue.Push(std::move(exit));
        m_thread.join();
    }

    LogThread(const LogThread&) = delete;
    LogThread& operator=(const LogThread&) = delete;

    void Send(LogMessage&& msg) {
        if (!msg.exited) {
            m_queue.Push(std::move(msg));
        }
    }

    void AddWriter(std::unique_ptr<LogWriter> writer) {
        m_writers.push_back(std::move(writer));
    }

private:
    LogQueue<LogMessage> m_queue;
    std::thread m_thread;
    std::vector<std::unique_ptr<LogWriter>> m_writers;

    void run() {
        while (true) {
            LogMessage msg;
            m_queue.Pop(&msg);
            if (msg.exited) {
                break;
            }
            write(msg);
        }
    }

    void write(const LogMessage& msg) {
        char level = toCharacter(msg.level);
        char timestamp[32];
        toString(msg.timestamp, timestamp, sizeof(timestamp));
        for (auto& writer : m_writers) {
            writer->Print(level, timestamp, msg);
        }
    }

    char toCharacter(LogLevel level) {
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

    void toString(const struct timeval& time, char* str, size_t size) {
        assert(size >= 25);

        time_t sec = time.tv_sec;
        struct tm calendar;
        localtime_r(&sec, &calendar);
        strftime(str, size, "%y-%m-%d %H:%M:%S", &calendar);
        const int offset = 17;
        snprintf(&str[offset], size - offset, ".%06ld", (long)time.tv_usec);
    }
};

struct StdoutLogWriter final : public LogWriter {
    void Print(char level, const char* timestamp, const LogMessage& msg) {
        // call printf 3 times to avoid a runtime error on Windows
        printf("%c %s %ld ", level, timestamp, msg.threadID);
        printf("%s:%d: ", msg.file, msg.line);
        printf("%s\n", msg.content.get());
    }
};

struct StderrLogWriter final : public LogWriter {
    void Print(char level, const char* timestamp, const LogMessage& msg) {
        // call fprintf 3 times to avoid a runtime error on Windows
        fprintf(stderr, "%c %s %ld ", level, timestamp, msg.threadID);
        fprintf(stderr, "%s:%d: ", msg.file, msg.line);
        fprintf(stderr, "%s\n", msg.content.get());
    }
};

class FileLogWriter final : public LogWriter {
public:
    FileLogWriter(const char* filename, int64_t maxFileSize, uint8_t maxBackupFiles)
            : m_filename(filename)
            , m_maxFileSize(maxFileSize > 0 ? maxFileSize : kDefaultMaxFileSize)
            , m_maxBackupFiles(maxBackupFiles)
            , m_output(nullptr)
            , m_currentFileSize(0) {}

    ~FileLogWriter() {
        if (m_output != nullptr) {
            fclose(m_output);
        }
    }

    bool Init() {
        m_output = fopen(m_filename.c_str(), "a");
        if (m_output == nullptr) {
            fprintf(stderr, "ERROR: logger: Failed to open file: `%s`\n", m_filename.c_str());
            return false;
        }
        m_currentFileSize = getFileSize(m_filename.c_str());
        return true;
    }

    void Print(char level, const char* timestamp, const LogMessage& msg) {
        if (m_output == nullptr) {
            return;
        }

        if (rotateLogFiles()) {
            // call fprintf 3 times to avoid a runtime error on Windows
            int size;
            if ((size = fprintf(m_output, "%c %s %ld ", level, timestamp, msg.threadID)) > 0) {
                m_currentFileSize += size;
            }
            if ((size = fprintf(m_output, "%s:%d: ", msg.file, msg.line)) > 0) {
                m_currentFileSize += size;
            }
            if ((size = fprintf(m_output, "%s\n", msg.content.get())) > 0) {
                m_currentFileSize += size;
            }
        }
    }

private:
    std::string m_filename;
    int64_t m_maxFileSize;
    uint8_t m_maxBackupFiles;
    FILE* m_output;
    int64_t m_currentFileSize;

    bool rotateLogFiles() {
        if (m_currentFileSize < m_maxFileSize) {
            return m_output != nullptr;
        }
        fclose(m_output);
        for (int i = (int)m_maxBackupFiles; i > 0; i--) {
            std::string src = getBackupFileName(m_filename, i - 1);
            std::string dst = getBackupFileName(m_filename, i);
            if (isFileExist(dst)) {
                if (remove(dst.c_str()) != 0) {
                    fprintf(stderr, "ERROR: logger: Failed to remove file: `%s`\n", dst.c_str());
                }
            }
            if (isFileExist(src)) {
                if (rename(src.c_str(), dst.c_str()) != 0) {
                    fprintf(stderr, "ERROR: logger: Failed to rename file: `%s` -> `%s`\n", src.c_str(), dst.c_str());
                }
            }
        }
        m_output = fopen(m_filename.c_str(), "a");
        if (m_output == nullptr) {
            fprintf(stderr, "ERROR: logger: Failed to open file: `%s`\n", m_filename.c_str());
            return false;
        }
        m_currentFileSize = getFileSize(m_filename);
        return true;
    }

    std::string getBackupFileName(const std::string& basename, uint8_t index) {
        if (index == 0) {
            return basename;
        }
        return basename + "." + std::to_string(index);
    }

    bool isFileExist(const std::string& filename) {
        FILE* fp;
        if ((fp = fopen(filename.c_str(), "r")) == nullptr) {
            return false;
        }
        fclose(fp);
        return true;
    }

    int64_t getFileSize(const std::string& filename) {
        std::ifstream stream(filename, std::ios::ate | std::ios::binary);
        return stream.tellg();
    }
};

} // namespace

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
