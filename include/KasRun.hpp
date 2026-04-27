#ifndef KASRUN_HPP
#define KASRUN_HPP

#include "KasLog.hpp"
#include "KasProjectConfig.hpp"

namespace KasRun {
    int run_command(std::map<std::string, KasProjectConfig::CommandConfig> &commands, const std::string &cmd_name);
}

#endif