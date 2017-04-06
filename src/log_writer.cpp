#include "log_writer.h"
#include <cstdio>

namespace log {

void StdoutLogWriter::Print(char level, const char* timestamp, const LogMessage& msg) {
    printf("%c %s %ld %s:%d: %s\n", level, timestamp,
            msg.threadID, msg.file, msg.line, msg.content);
}

void StderrLogWriter::Print(char level, const char* timestamp, const LogMessage& msg) {
    fprintf(stderr, "%c %s %ld %s:%d: %s\n", level, timestamp,
            msg.threadID, msg.file, msg.line, msg.content);
}

FileLogWriter::FileLogWriter(const char* filename)
        : m_filename(filename), m_output(nullptr) {}

FileLogWriter::~FileLogWriter() {
    if (m_output != nullptr) {
        fclose(m_output);
    }
}

bool FileLogWriter::Init() {
    m_output = fopen(m_filename.c_str(), "a");
    if (m_output == nullptr) {
        fprintf(stderr, "ERROR: logger: Failed to open file: `%s`\n", m_filename.c_str());
        return false;
    }
    return true;
}

void FileLogWriter::Print(char level, const char* timestamp, const LogMessage& msg) {
    if (m_output != nullptr) {
        fprintf(m_output, "%c %s %ld %s:%d: %s\n", level, timestamp,
                msg.threadID, msg.file, msg.line, msg.content);
    }
}

} // namespace log
