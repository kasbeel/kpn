#include <KasLog.hpp>

namespace KasLog {
    void log(Level level, const char *fmt, ...) {
        bool is_terminal = isatty(STDOUT_FILENO);

        const char *label = "INFO";
        const char *color = "";
        const char *reset = "";

        if (is_terminal) {
            reset = "\033[0m";
            switch (level) {
            case Level::DEBUG:
                color = "\033[36m";
                label = "DEBUG";
                break;
            case Level::INFO:
                color = "\033[32m";
                label = "INFO ";
                break;
            case Level::WARN:
                color = "\033[33m";
                label = "WARN ";
                break;
            case Level::ERR:
                color = "\033[31m";
                label = "ERROR";
                break;
            }
        } else {
            switch (level) {
            case Level::DEBUG:
                label = "DEBUG";
                break;
            case Level::INFO:
                label = "INFO";
                break;
            case Level::WARN:
                label = "WARN";
                break;
            case Level::ERR:
                label = "ERROR";
                break;
            }
        }

        char message[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(message, sizeof(message), fmt, args);
        va_end(args);
        printf("%s[%s]%s %s\n", color, label, reset, message);
    }
} // namespace KasLog