#ifndef KASLOG_HPP
#define KASLOG_HPP

#include <cstdarg>
#include <cstdio>
#include <unistd.h>

namespace KasLog {
    enum class Level {
        DEBUG,
        INFO,
        WARN,
        ERR
    };

    void log(Level level, const char *fmt, ...);
} // namespace KasLog

#endif