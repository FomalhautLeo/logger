#pragma once

#include <stdint.h>
#include <sys/types.h>

namespace logger {

namespace config {

// constexpr char*  kSyncerFileName   = "/trading-system/log/log_syncer.log";
// constexpr char*  kSockFileName     = "/trading-system/log/log_socket.sock";
constexpr char*      kSyncerFileName   = "/home/jace/test/logger/log_syncer.log";
constexpr char*      kSockFileName     = "/home/jace/test/logger/log_socket.sock";
constexpr size_t kMsgBufSize       = 512;
constexpr size_t kMaxSessionCnt    = 128;
constexpr size_t kSyncerFileSize   = 134217728;                         // 128MB
constexpr size_t kSessionBlockSize = kSyncerFileSize / kMaxSessionCnt;  // 1MB 
constexpr size_t kMsgPerBlock      = kSessionBlockSize / kMsgBufSize;   // 2048


}  // namespace config

namespace comm {  // communication

constexpr char   kSuccess  = 'A';        // 请求成功
constexpr char   kRegister = 'R';        // 注册请求

}  // namespace comm

constexpr int DEBUG = 0;
constexpr int INFO  = 1;
constexpr int WARN  = 2;
constexpr int ERROR = 3;

constexpr char* levels[] {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR"
};

}  // namespace logger