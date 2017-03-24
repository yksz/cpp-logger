#include <chrono>
#include <thread>
#include "logger.h"

int main(void) {
    using namespace log;

    Logger::Init();
    auto& logger = Logger::Instance();
    logger.Log(LogLevel::WARN,  "a");
    logger.Log(LogLevel::INFO,  "b");
    logger.Log(LogLevel::DEBUG, "c");
    return 0;
}
