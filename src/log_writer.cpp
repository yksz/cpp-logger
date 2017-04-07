#include "log_writer.h"
#include <cstdio>

namespace logger {

void StdoutLogWriter::Print(char level, const char* timestamp, const LogMessage& msg) {
    // call printf 3 times to avoid a runtime error on Windows
    printf("%c %s %ld ", level, timestamp, msg.threadID);
    printf("%s:%d: ", msg.file, msg.line);
    printf("%s\n", msg.content);
}

void StderrLogWriter::Print(char level, const char* timestamp, const LogMessage& msg) {
    // call fprintf 3 times to avoid a runtime error on Windows
    fprintf(stderr, "%c %s %ld ", level, timestamp, msg.threadID);
    fprintf(stderr, "%s:%d: ", msg.file, msg.line);
    fprintf(stderr, "%s\n", msg.content);
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
        // call fprintf 3 times to avoid a runtime error on Windows
        fprintf(m_output, "%c %s %ld ", level, timestamp, msg.threadID);
        fprintf(m_output, "%s:%d: ", msg.file, msg.line);
        fprintf(m_output, "%s\n", msg.content);
    }
}

} // namespace logger
