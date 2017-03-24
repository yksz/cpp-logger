#include <chrono>
#include <thread>
#include "logger.h"

int main(void) {
    log::Logger::Init();
    auto& logger = log::Logger::Instance();
    logger.Log("a");
    logger.Log("b");
    logger.Log("c");
    return 0;
}
