#pragma once

#include "include/log_config.h"
#include "include/service.h"
#include "include/sys.h"

namespace logger {
    
class Logger {

public:
    static Logger* GetInstance() {
        static Logger instance;
        return &instance;
    }

    void Register(const char* log_file_name);

    template <typename... T>
    void Debug(fmt::format_string<T...> format, T&&... args) {
        Log("DEBUG", format, (T&&)args...);
    }

    template <typename... T>
    void Info(fmt::format_string<T...> format, T&&... args) {
        Log("INFO ", format, (T&&)args...);
    }
    template <typename... T>
    void Warn(fmt::format_string<T...> format, T&&... args) {
        Log("WARN ", format, (T&&)args...);
    }
    template <typename... T>
    void Error(fmt::format_string<T...> format, T&&... args) {
        Log("ERROR", format, (T&&)args...);
    }

private:
    Logger() : session_num_(0), item_num_(nullptr), log_ptr_(nullptr) { }
    Logger(const Logger& other) = delete;

    void GetLogPtr();

    template <typename... T>
    void Log(const char* level, fmt::format_string<T...> format, T&&... args) {
        bzero(log_buf_, config::kMsgBufSize);
        Snprintf(log_buf_, config::kMsgBufSize, format, (T&&)args...);
        char tmp_buf[config::kMsgBufSize];
        bzero(tmp_buf, config::kMsgBufSize);
        Snprintf(tmp_buf, config::kMsgBufSize, "[{}][{}][文件名...] {}\n", TimeStrNow(), level, log_buf_);
        memory::wfence();
        memcpy(log_ptr_ + *item_num_ % config::kMsgPerBlock * config::kMsgBufSize,
            tmp_buf,
            config::kMsgBufSize);
        (*item_num_)++;

    }

    uint64_t session_num_;
    volatile uint64_t* item_num_;
    char* log_ptr_;
    char log_buf_[config::kMsgBufSize];
};

}  // namespace logger