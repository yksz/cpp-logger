#include "logger.h"
#include "log_thread.h"

namespace log {

void Logger::Init() {
    auto thread = Logger::Instance().Thread();
    thread->AddWriter(std::unique_ptr<StdoutLogWriter>(new StdoutLogWriter()));
    thread->AddWriter(std::unique_ptr<StderrLogWriter>(new StderrLogWriter()));
}

Logger::Logger() {
    m_thread = std::make_shared<LogThread>();
}

Logger::~Logger() {}

void Logger::Log(std::string& msg) {
    m_thread->Send(std::string(msg));
}

void Logger::Log(std::string&& msg) {
    m_thread->Send(std::move(msg));
}

} // namespace log
