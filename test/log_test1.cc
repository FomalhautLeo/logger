#include "include/logger.h"
#include <unistd.h>

using namespace logger;

int main() {
    auto logger = Logger::GetInstance();    
    logger->Register("log1.log");

    int cur = -16384;
    for (int i = 0; i < 1000; i++) {
        logger->Warn("log test: {}", cur++);
        usleep(800000);
    }
    return 0;
}