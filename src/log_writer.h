#pragma once

#include <string>

namespace log {

struct LogWriter {
    virtual ~LogWriter() {}
    virtual void Print(const std::string& msg) = 0;
};

struct StdoutLogWriter final : public LogWriter {
    void Print(const std::string& msg);
};

struct StderrLogWriter final : public LogWriter {
    void Print(const std::string& msg);
};

class FileLogWriter final : public LogWriter {
public:
    FileLogWriter(const char* filename);
    ~FileLogWriter();
    bool Init();
    void Print(const std::string& msg);

private:
    std::string m_filename;
    FILE* m_output;
};

} // namespace log
