#pragma once

#include <string>

namespace log {

struct LogWriter {
    virtual ~LogWriter() {}
    virtual void Println(const std::string& msg) = 0;
};

struct StdoutLogWriter final : public LogWriter {
    void Println(const std::string& msg);
};

struct StderrLogWriter final : public LogWriter {
    void Println(const std::string& msg);
};

} // namespace log
