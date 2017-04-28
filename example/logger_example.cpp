#include <thread>
#include "logger.h"

int main(void) {
    using namespace logger;

    Logger::SetLevel(LogLevel_INFO);
    Logger::InitConsoleLogger(stderr);
    Logger::InitFileLogger("log.txt", 0, 10);
    std::thread th([] {
        LOG_ERROR("%d", 0);
    });
    th.join();
    LOG_WARN("%s", "1");
    LOG_INFO("%c", '2');
    LOG_DEBUG("%d", 3);
    return 0;
}
