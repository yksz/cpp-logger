#include <chrono>
#include <thread>
#include "logger.h"

int main(void) {
    log::Logger logger;
    logger.Println("a");
    logger.Println("b");
    logger.Println("c");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
