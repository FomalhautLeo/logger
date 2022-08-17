#include "include/log_server.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <iostream>
#include <thread>

#include "include/log_config.h"
#include "include/sys.h"

namespace logger {

// LogServer::instance_ = nullptr;
void LogServer::Start(LogServer::StartMode start_mode) {
    if (start_mode == StartMode::ASYNC) {
        std::thread thd(&LogServer::ServerLoop, this);
        thd.detach();
    } else if (start_mode == StartMode::SYNC) {
        ServerLoop();
    } else {
        std::cerr << "Please pass a correct start mode!" << std::endl;
        exit(1);
    }
}

void LogServer::ServerLoop() {
    int syncer_fd = open(config::kSyncerFileName, O_RDWR | O_CREAT | O_TRUNC, 0666); if (syncer_fd < 0) {
        perror("Open syncer file");
        exit(1);
    }

    if (ftruncate64(syncer_fd, config::kSyncerFileSize) < 0) {
        perror("Truncate syncer file");
        exit(1);
    }

    char* addr = (char*)mmap(nullptr,
                             config::kSyncerFileSize,
                             PROT_READ | PROT_WRITE,
                             MAP_SHARED,
                             syncer_fd,
                             0);
    close(syncer_fd);
    if (addr == MAP_FAILED) {
        perror("Mmap syncer file");
        exit(1);
    }
    // 第0个block为头部，第1个block为消息起始位置
    item_cnts_ = (uint64_t*)addr;
    msg_ptrs_[0] = addr + config::kSessionBlockSize;
    
    // 创建 UNIX domain socket
    sockaddr_un server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    memcpy(server_addr.sun_path, config::kSockFileName, sizeof(server_addr.sun_path) - 1);

    if (remove(config::kSockFileName) < 0 && errno != ENOENT) {
        perror("Remove sock file");
        exit(1);
    }

    int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Create server socket");
        exit(1);
    }

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        perror("Bind unix domain socket");
        exit(1);
    }

    if (listen(server_socket, 128)) {
        perror("Listen on the socket");
        exit(1);
    }
    
    // 启动 syncer
    std::thread thd(&LogServer::SyncLog, this);
    thd.detach();

    // 接收新的注册请求
    char msg_buf[BUFSIZ];
    while (true) {
        int client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket < 0) {
            perror("Accept client socket");
            break;
        }

        bzero(msg_buf, BUFSIZ);
        int msg_len = recv(client_socket, msg_buf, BUFSIZ, 0);
        if (msg_len > 0) {
            ParseRequest(msg_buf);
            if (send(client_socket, msg_buf, BUFSIZ, 0) < 0) {
                perror("Send response");
            }
        }
        close(client_socket);
    }
    close(server_socket);
}

void LogServer::ParseRequest(char* msg_buf) {
    switch(msg_buf[0]) {
        case comm::kRegister: {
            int session_fd = open(&msg_buf[1], O_CREAT | O_RDWR | O_APPEND, 0666);
            if (session_fd < 0) {
                perror("Open log file");
                exit(1);
            }

            // 编辑返回消息
            bzero(msg_buf, BUFSIZ);
            msg_buf[0] = comm::kSuccess;
            // 对应的session序号
            memcpy(&msg_buf[1], &session_cnt_, sizeof(session_cnt_));

            // 新注册 seesion
            prev_items_[session_cnt_] = 0;
            msg_ptrs_[session_cnt_] = msg_ptrs_[0] + session_cnt_ * config::kSessionBlockSize;
            session_fds_[session_cnt_++] = session_fd;
            break;
        }
        default: {
            std::cerr << "Wrong message type!" << std::endl;
        }
    }
}

void LogServer::SyncLog() {
    while (true) {
        // 轮询所有session，查看是否有更新
        for (uint64_t i = 0; i < session_cnt_; i++) {
            memory::rfence();
            if (item_cnts_[i] != prev_items_[i]) {
                // 消息有更新
                char* msg_pos = msg_ptrs_[i] + prev_items_[i] % config::kMsgPerBlock * config::kMsgBufSize;
                write(session_fds_[i],
                      msg_pos,
                      strlen(msg_pos));
                prev_items_[i]++;
            }
        }
    }
}

}  // namespace logger