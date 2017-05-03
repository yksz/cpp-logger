#include "loggerconf.h"
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <string>
#include "logger.h"

namespace logger {

// Logger type
static const int kConsoleLogger = 1 << 0;
static const int kFileLogger = 1 << 1;

namespace {

struct config {
    int loggerType;
    FILE* output;
    std::string filename;
    int64_t maxFileSize;
    uint8_t maxBackupFiles;
};

} // namespace

static void removeComments(std::string& s);
static void trim(std::string& s);
static void parseLine(std::string& line, config* conf);
static bool hasFlag(int flags, int flag);

bool Configure(const char* filename) {
    if (filename == nullptr) {
        assert(0 && "filename must not be nullptr");
        return false;
    }

    std::ifstream stream(filename);
    if (stream.fail()) {
        fprintf(stderr, "ERROR: loggerconf: Failed to open file: `%s`\n", filename);
        return false;
    }
    config conf;
    std::string line;
    while (std::getline(stream, line)) {
        removeComments(line);
        trim(line);
        if (line.empty()) {
            continue;
        }
        parseLine(line, &conf);
    }

    if (hasFlag(conf.loggerType, kConsoleLogger)) {
        if (!Logger::InitConsoleLogger(conf.output)) {
            return false;
        }
    }
    if (hasFlag(conf.loggerType, kFileLogger)) {
        if (!Logger::InitFileLogger(conf.filename.c_str(), conf.maxFileSize, conf.maxBackupFiles)) {
            return false;
        }
    }
    if (conf.loggerType == 0) {
        return false;
    }
    return true;
}

static void removeComments(std::string& s) {
    auto pos = s.find("#");
    if (pos != std::string::npos) {
        s.erase(pos);
    }
}

static void trim(std::string& s) {
    const char* spaces = " \t\n\v\f\r";

    if (s.empty()) {
        return;
    }
    // trim right
    auto rpos = s.find_last_not_of(spaces);
    if (rpos != std::string::npos) {
        s.erase(rpos + 1);
    }
    // trim left
    auto lpos = s.find_first_not_of(spaces);
    if (lpos != std::string::npos) {
        s.erase(0, lpos);
    }
}

static LogLevel parseLevel(const std::string& s);

static void parseLine(std::string& line, config* conf) {
    auto pos = line.find("=");
    std::string key = line.substr(0, pos);
    std::string val = line.substr(pos + 1);

    if (key == "level") {
        LogLevel level = parseLevel(val);
        Logger::SetLevel(level);
    } else if (key == "logger") {
        if (val == "console") {
            conf->loggerType |= kConsoleLogger;
        } else if (val == "file") {
            conf->loggerType |= kFileLogger;
        } else {
            fprintf(stderr, "ERROR: loggerconf: Invalid logger: `%s`\n", val.c_str());
        }
    } else if (key == "logger.console.output") {
        if (val == "stdout") {
            conf->output = stdout;
        } else if (val == "stderr") {
            conf->output = stderr;
        } else {
            fprintf(stderr, "ERROR: loggerconf: Invalid logger.console.output: `%s`\n", val.c_str());
        }
    } else if (key == "logger.file.filename") {
        conf->filename = val;
    } else if (key == "logger.file.maxFileSize") {
        conf->maxFileSize = atol(val.c_str());
    } else if (key == "logger.file.maxBackupFiles") {
        int nfiles = atoi(val.c_str());
        if (nfiles < 0) {
            fprintf(stderr, "ERROR: loggerconf: Invalid logger.file.maxBackupFiles: `%s`\n", val.c_str());
            nfiles = 0;
        }
        conf->maxBackupFiles = (uint8_t) nfiles;
    }
}

static LogLevel parseLevel(const std::string& s) {
    if (s == "TRACE") {
        return LogLevel_TRACE;
    } else if (s == "DEBUG") {
        return LogLevel_DEBUG;
    } else if (s == "INFO") {
        return LogLevel_INFO;
    } else if (s == "WARN") {
        return LogLevel_WARN;
    } else if (s == "ERROR") {
        return LogLevel_ERROR;
    } else if (s == "FATAL") {
        return LogLevel_FATAL;
    } else {
        fprintf(stderr, "ERROR: loggerconf: Invalid level: `%s`\n", s.c_str());
        return Logger::Level();
    }
}

static bool hasFlag(int flags, int flag) {
    return (flags & flag) == flag;
}

} // namespace logger
