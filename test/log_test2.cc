#include "include/logger.h"
#include <unistd.h>

using namespace logger;

int main() {
    auto logger = Logger::GetInstance();    
    logger->Register("log2.log");

    int cur = 0;
    for (int i = 0; i < 1000; i++) {
        logger->Debug("{}", cur++);
        usleep(500000);
        logger->Info("{:.2f}", (double)cur++);
        usleep(500000);
    }
    return 0;
}