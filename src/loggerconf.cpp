#include "loggerconf.h"
#include <cassert>
#include <fstream>
#include <string>
#include "logger.h"

namespace logger {

// Logger type
static const int kConsoleLogger = 1 << 0;
static const int kFileLogger = 1 << 1;

static int s_logger;

static void removeComments(std::string& s);
static void trim(std::string& s);
static void parseLine(std::string& line);

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
    std::string line;
    while (std::getline(stream, line)) {
        removeComments(line);
        trim(line);
        if (line.empty()) {
            continue;
        }
        parseLine(line);
    }
    s_logger = 0;
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
static bool hasFlag(int flags, int flag);

static void parseLine(std::string& line) {
    auto pos = line.find("=");
    std::string key = line.substr(0, pos);
    std::string val = line.substr(pos + 1);

    if (key == "level") {
        LogLevel level = parseLevel(val);
        Logger::SetLevel(level);
    } else if (key == "logger") {
        if (val == "console") {
            s_logger |= kConsoleLogger;
        } else if (val == "file") {
            s_logger |= kFileLogger;
        } else {
            fprintf(stderr, "ERROR: loggerconf: Invalid logger: `%s`\n", val.c_str());
        }
    } else if (key == "logger.console.output") {
        if (hasFlag(s_logger, kConsoleLogger)) {
            if (val == "stdout") {
                Logger::InitConsoleLogger(stdout);
            } else if (val == "stderr") {
                Logger::InitConsoleLogger(stderr);
            } else {
                fprintf(stderr, "ERROR: loggerconf: Invalid logger.console.output: `%s`\n", val.c_str());
            }
        }
    } else if (key == "logger.file.filename") {
        if (hasFlag(s_logger, kFileLogger)) {
            Logger::InitFileLogger(val.c_str());
        }
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
