#include "logger.h"

static const int kLoggingCount = 1000000;

int main(void) {
    log::Logger::InitFileLogger("logs/logger.txt");
    for (int i = 0; i < kLoggingCount; i++) {
        LOG_INFO("%d", i);
    }
    return 0;
}
