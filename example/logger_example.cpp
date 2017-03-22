#include <chrono>
#include <thread>
#include "logger.h"

int main(void) {
    log::Logger logger;
    logger.Println("a");
    logger.Println("b");
    logger.Println("c");
    return 0;
}
