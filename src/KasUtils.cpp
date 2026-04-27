#include <KasUtils.hpp>

namespace KasUtils {
    void append_spaced(std::string &dst, const std::string &value) {
        if (!value.empty()) {
            dst += value + " ";
        }
    }

    std::string join_flags(const std::vector<std::string> &flags) {
        std::string result;
        for (const auto &flag : flags)
            append_spaced(result, flag);
        return result;
    }

    void ensure_flag(std::string &flags, const std::string &flag) {
        if (flags.find(flag + " ") == std::string::npos) {
            append_spaced(flags, flag);
        }
    }

    std::string exec_cmd(const std::string &cmd) {
        char buffer[128];
        std::string result;
        FILE *pipe = popen(cmd.c_str(), "r");
        if (!pipe)
            return "ERROR";

        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
            result += buffer;

        pclose(pipe);
        if (!result.empty() && result.back() == '\n')
            result.pop_back();

        return result;
    }
} // namespace KasUtils