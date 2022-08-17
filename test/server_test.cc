#include "include/log_server.h"

using namespace logger;
int main() {
    auto server = LogServer::GetInstance();
    server->Start(LogServer::StartMode::SYNC);
    return 0;
}