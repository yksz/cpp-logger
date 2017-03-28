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

} // namespace log
