#pragma once

#include <string.h>

#include <iostream>

#include "dep/fmt/include/core.h"
#include "dep/fmt/include/format.h"

namespace logger {

    inline const char* TimeStrNow(const char* fmt = "%Y-%m-%d %H:%M:%S") {
        const size_t buf_size = 64;
        static char buf[buf_size];
        bzero(buf, buf_size);

        time_t time_stamp;
        time(&time_stamp);
        strftime(buf, buf_size, fmt, localtime(&time_stamp));
        return buf;
    }

    template <typename... T>
    void Sprintf(char* dst, fmt::format_string<T...> fmt, T&&... args) {
        fmt::vformat_to(dst, fmt, fmt::make_format_args(args...));
    }

    template <typename... T>
    void Snprintf(char* dst, size_t len, fmt::format_string<T...> fmt, T&&... args) {
        fmt::vformat_to_n(dst, len, fmt, fmt::make_format_args(args...));
    }

}  // namespace logger