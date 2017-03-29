#include "log_writer.h"
#include <cstdio>

namespace log {

void StdoutLogWriter::Print(const std::string& msg) {
    printf("%s", msg.c_str());
}

void StderrLogWriter::Print(const std::string& msg) {
    fprintf(stderr, "%s", msg.c_str());
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

void FileLogWriter::Print(const std::string& msg) {
    if (m_output != nullptr) {
        fprintf(m_output, "%s", msg.c_str());
    }
}

} // namespace log
