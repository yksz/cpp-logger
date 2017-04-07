#pragma once

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
    FileLogWriter(const char* filename);
    ~FileLogWriter();
    bool Init();
    void Print(char level, const char* timestamp, const LogMessage& msg);

private:
    std::string m_filename;
    FILE* m_output;
};

} // namespace logger
