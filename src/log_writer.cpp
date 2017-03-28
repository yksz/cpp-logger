#include "log_writer.h"
#include <cstdio>

namespace log {

void StdoutLogWriter::Print(const std::string& msg) {
    printf("%s", msg.c_str());
}

void StderrLogWriter::Print(const std::string& msg) {
    fprintf(stderr, "%s", msg.c_str());
}

} // namespace log
