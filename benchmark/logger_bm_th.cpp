#include <cstdlib>
#include <atomic>
#include <thread>
#include <vector>
#include "logger.h"

static const int kLoggingCount = 1000000;

int main(int argc, char** argv) {
    int nThreads = 10;
    if (argc > 1) {
        nThreads = atoi(argv[1]);
    }

    logger::Logger::InitFileLogger("logs/logger.txt");

    std::atomic<int> count(0);
    std::vector<std::thread> threads;
    for (int i = 0; i < nThreads; i++) {
        threads.push_back(std::thread([&]() {
            while (count < kLoggingCount) {
                LOG_INFO("%d", count++);
            }
        }));
    }
    for (std::thread& th : threads) {
        th.join();
    }
    return 0;
}
