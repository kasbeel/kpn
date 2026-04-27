#ifndef KASUTILS_HPP
#define KASUTILS_HPP
#include "KasLog.hpp"
#include <string>
#include <vector>

namespace KasUtils {
    void append_spaced(std::string &dst, const std::string &value);

    std::string join_flags(const std::vector<std::string> &flags);

    void ensure_flag(std::string &flags, const std::string &flag);

    std::string exec_cmd(const std::string &cmd);
} // namespace KasUtils
#endif // KASUTILS_HPP