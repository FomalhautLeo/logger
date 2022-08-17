#include "include/logger.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <iostream>

namespace logger {

void Logger::Register(const char* log_file_name) {
    // 向server注册新会话
    sockaddr_un server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    memcpy(server_addr.sun_path, config::kSockFileName, sizeof(server_addr.sun_path) - 1);

    int client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Create client socket");
        exit(1);
    }

    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect to server");
        exit(1);
    }

    // 编辑请求消息
    char msg_buf[BUFSIZ];
    bzero(msg_buf, BUFSIZ);
    msg_buf[0] = comm::kRegister;
    memcpy(&msg_buf[1], log_file_name, strlen(log_file_name));

    if (send(client_socket, msg_buf, BUFSIZ, 0) < 0) {
        perror("Send request message");
        exit(1);
    }

    int recv_len = recv(client_socket, msg_buf, BUFSIZ, 0);
    if (recv_len < 0) {
        perror("Recv response message");
        exit(1);
    }

    switch (msg_buf[0]) {
        case comm::kSuccess: {
            // 获取会话序号
            memcpy(&session_num_, &msg_buf[1], sizeof(session_num_));
            this->GetLogPtr();
            break;
        }

        default: {
            std::cerr << "Unkonwn message type!" << std::endl;
            exit(1);
        }
    }
}

void Logger::GetLogPtr() {
    int syncer_fd = open(config::kSyncerFileName, O_RDWR, 0666);
    if (syncer_fd < 0) {
        perror("Open syncer file");
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

    // 获取对应session记录的区域
    item_num_ = (uint64_t*)addr;
    item_num_ += session_num_;

    // 获取对应session的log指针
    log_ptr_ = addr + ((session_num_ + 1)* config::kSessionBlockSize);
}

}  // namespace logger