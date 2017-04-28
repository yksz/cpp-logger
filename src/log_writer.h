#pragma once

#include <cstdint>
#include <string>
#include "log_message.h"

namespace logger {

struct LogWriter {
    virtual ~LogWriter() {}
    virtual void Print(char level, const char* timestamp, const LogMessage& msg) = 0;
};

struct StdoutLogWriter final : public LogWriter {
    void Print(char level, const char* timestamp, const LogMessage& msg);
};

struct StderrLogWriter final : public LogWriter {
    void Print(char level, const char* timestamp, const LogMessage& msg);
};

class FileLogWriter final : public LogWriter {
public:
    FileLogWriter(const char* filename, int64_t maxFileSize, uint8_t maxBackupFiles);
    ~FileLogWriter();
    bool Init();
    void Print(char level, const char* timestamp, const LogMessage& msg);

private:
    std::string m_filename;
    int64_t m_maxFileSize;
    uint8_t m_maxBackupFiles;
    FILE* m_output;
    int64_t m_currentFileSize;

    bool rotateLogFiles();
};

} // namespace logger
