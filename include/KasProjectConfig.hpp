#ifndef KASPROJECTCONFIG_HPP
#define KASPROJECTCONFIG_HPP

#include <json.hpp>
#include <map>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace KasProjectConfig {
    struct WarningsConfig {
        std::string level;
        bool extra;
        bool pedantic;
    };

    struct ToolchainConfig {
        std::string compiler;
        std::map<std::string, std::string> standards;
        WarningsConfig warnings;
        int jobs;
        std::string sysroot;
        std::vector<std::string> cxxflags;
        std::vector<std::string> ldflags;
    };

    struct ProfileConfig {
        std::vector<std::string> defines;
        std::vector<std::string> cxxflags;
        bool lto;
        bool strip;
    };

    struct TargetConfig {
        std::string name;
        std::string type;
        std::vector<std::string> sources;
        std::vector<std::string> includes;
        std::vector<std::string> system_libs;
        std::vector<std::string> dependencies;
        std::string output_name;
        std::string output_extension;
        std::string build_dir;
        std::string output_dir;
        std::vector<std::string> cxxflags;
        std::vector<std::string> ldflags;
    };

    struct CommandConfig {
        bool enabled;
        std::string command;
    };

    struct KasProject {
        std::string schema_version;
        std::string project_name;
        std::string version;
        std::string description;
        std::string license;

        ToolchainConfig toolchain;

        std::map<std::string, ProfileConfig> profiles;
        std::vector<TargetConfig> targets;
        std::map<std::string, CommandConfig> commands;
    };
    void from_json(const json &j, WarningsConfig &w);
    void from_json(const json &j, ToolchainConfig &t);
    void from_json(const json &j, ProfileConfig &p);
    void from_json(const json &j, TargetConfig &t);
    void from_json(const json &j, CommandConfig &c);
    void from_json(const json &j, KasProject &p);
} // namespace KasProjectConfig

#endif