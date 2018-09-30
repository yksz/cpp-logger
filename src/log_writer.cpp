#include "log_writer.h"
#include <cstdio>
#include <fstream>

namespace logger {

void StdoutLogWriter::Print(char level, const char* timestamp, const LogMessage& msg) {
    // call printf 3 times to avoid a runtime error on Windows
    printf("%c %s %ld ", level, timestamp, msg.threadID);
    printf("%s:%d: ", msg.file, msg.line);
    printf("%s\n", msg.content.get());
}

void StderrLogWriter::Print(char level, const char* timestamp, const LogMessage& msg) {
    // call fprintf 3 times to avoid a runtime error on Windows
    fprintf(stderr, "%c %s %ld ", level, timestamp, msg.threadID);
    fprintf(stderr, "%s:%d: ", msg.file, msg.line);
    fprintf(stderr, "%s\n", msg.content.get());
}

static const int64_t kDefaultMaxFileSize = 1048576L; // 1 MB

FileLogWriter::FileLogWriter(const char* filename, int64_t maxFileSize, uint8_t maxBackupFiles)
        : m_filename(filename),
          m_maxFileSize(maxFileSize > 0 ? maxFileSize : kDefaultMaxFileSize),
          m_maxBackupFiles(maxBackupFiles),
          m_output(nullptr), m_currentFileSize(0) {}

FileLogWriter::~FileLogWriter() {
    if (m_output != nullptr) {
        fclose(m_output);
    }
}

static int64_t getFileSize(const std::string& filename);

bool FileLogWriter::Init() {
    m_output = fopen(m_filename.c_str(), "a");
    if (m_output == nullptr) {
        fprintf(stderr, "ERROR: logger: Failed to open file: `%s`\n", m_filename.c_str());
        return false;
    }
    m_currentFileSize = getFileSize(m_filename.c_str());
    return true;
}

static int64_t getFileSize(const std::string& filename) {
    std::ifstream stream(filename, std::ios::ate | std::ios::binary);
    return stream.tellg();
}

void FileLogWriter::Print(char level, const char* timestamp, const LogMessage& msg) {
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

static std::string getBackupFileName(const std::string& basename, uint8_t index);
static bool isFileExist(const std::string& filename);

bool FileLogWriter::rotateLogFiles() {
    if (m_currentFileSize < m_maxFileSize) {
        return m_output != nullptr;
    }
    fclose(m_output);
    for (int i = (int) m_maxBackupFiles; i > 0; i--) {
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

static bool isFileExist(const std::string& filename) {
    FILE* fp;
    if ((fp = fopen(filename.c_str(), "r")) == nullptr) {
        return false;
    }
    fclose(fp);
    return true;
}

static std::string getBackupFileName(const std::string& basename, uint8_t index) {
    if (index == 0) {
        return basename;
    }
    return basename + "." + std::to_string(index);
}

} // namespace logger
