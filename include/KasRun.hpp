#ifndef KASRUN_HPP
#define KASRUN_HPP

#include "KasLog.hpp"
#include "KasProjectConfig.hpp"

using namespace KasProjectConfig;

namespace KasRun {
    int run_command(std::map<std::string, CommandConfig> &commands, const std::string &cmd_name) {
        auto it = commands.find(cmd_name);
        if (it == commands.end()) {
            KasLog::log(KasLog::Level::ERR, "Command not found: %s", cmd_name.c_str());
            return 1;
        }
        CommandConfig &cmd = it->second;
        if (!cmd.enabled) {
            KasLog::log(KasLog::Level::WARN, "Command is disabled, skipping: %s", cmd.command.c_str());
            return 0;
        }
        KasLog::log(KasLog::Level::INFO, "Running command: %s", cmd.command.c_str());
        int res = system(cmd.command.c_str());
        if (res != 0) {
            KasLog::log(KasLog::Level::ERR, "Command failed with code %d: %s", res, cmd.command.c_str());
        }
        return res;
    }
} // namespace KasRun

#endif // KASRUN_HPP