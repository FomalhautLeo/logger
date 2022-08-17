#pragma once

#include <stdint.h>

#include "include/log_config.h"

namespace logger {

class LogServer {

public:
    static LogServer* GetInstance() {
        static LogServer instance;
        return &instance;
    }
    
    // LogServer 的启动模式
    enum StartMode {
        SYNC,  // 使用本进程作为 LogServer
        ASYNC  // 另起线程作为 LogServer
    };

    void Start(StartMode start_mode);

private:
    LogServer() : session_cnt_(0) { }
    LogServer(const LogServer& other) = delete;

    void ServerLoop();
    // 解析新的注册请求，直接原地修改 msg_buf 为应答字符串
    void ParseRequest(char* msg_buf);  
    // 同步其他进程写入的内容至文件
    void SyncLog();

    // 统计各进程的消息数量，并用于syncer检查是否有消息更新
    volatile uint64_t* item_cnts_;
    // 旧有消息数量
    uint64_t prev_items_[config::kMaxSessionCnt];
    // 各进程消息的具体位置
    char* msg_ptrs_[config::kMaxSessionCnt];
    // 当前会话数量
    uint64_t session_cnt_;
    // 各进程日志文件的描述符
    int session_fds_[config::kMaxSessionCnt];
};

}  // namespace logger