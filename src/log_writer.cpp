#include "log_writer.h"
#include <cstdio>

namespace log {

void StdoutLogWriter::Println(const std::string& msg) {
    printf("%s\n", msg.c_str());
}

void StderrLogWriter::Println(const std::string& msg) {
    fprintf(stderr, "%s\n", msg.c_str());
}

} // namespace log
