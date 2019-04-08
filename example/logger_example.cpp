#include <thread>
#include "logger.h"

int main(void) {
    logger::SetLevel(logger::LogLevel_INFO);
    logger::InitConsoleLogger(stderr);
    logger::InitFileLogger("log.txt", 0, 10);
    std::thread th([] {
        LOG_ERROR("%d", 0);
    });
    th.join();
    LOG_WARN("%s", "1");
    LOG_INFO("%c", '2');
    LOG_DEBUG("%d", 3);
    return 0;
}
