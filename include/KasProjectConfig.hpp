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
    // Macros de mapeo automático
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WarningsConfig, level, extra, pedantic)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ToolchainConfig, compiler, standards, warnings, jobs, sysroot, cxxflags, ldflags)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ProfileConfig, defines, cxxflags, lto, strip)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TargetConfig, name, type, sources, includes, system_libs, dependencies, output_name, build_dir, output_dir, cxxflags, ldflags)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandConfig, enabled, command)

    // Para KasProject mapeamos manualmente los que tienen nombres anidados o especiales
    void from_json(const json &j, KasProject &p) {
        p.schema_version = j.at("schema_version").get<std::string>();
        p.project_name = j.at("project").get<std::string>();
        p.version = j.at("version").get<std::string>();
        p.description = j.at("description").get<std::string>();
        p.license = j.at("license").get<std::string>();
        p.toolchain = j.at("toolchain").get<ToolchainConfig>();

        p.profiles = j.at("profiles").get<std::map<std::string, ProfileConfig>>();
        p.targets = j.at("targets").get<std::vector<TargetConfig>>();
        p.commands = j.at("commands").get<std::map<std::string, CommandConfig>>();
    }
    void from_json(const json &j, ToolchainConfig &t) {
        t.compiler = j.at("compiler").get<std::string>();
        t.standards = j.at("standards").get<std::map<std::string, std::string>>();
        t.warnings = j.at("warnings").get<WarningsConfig>();
        t.jobs = j.at("jobs").get<int>();
        t.sysroot = j.value("sysroot", std::string{});
        t.cxxflags = j.value("cxxflags", std::vector<std::string>{});
        t.ldflags = j.value("ldflags", std::vector<std::string>{});
    }
    void from_json(const json &j, TargetConfig &t) {
        t.name = j.at("name").get<std::string>();
        t.type = j.at("type").get<std::string>();
        t.sources = j.at("sources").get<std::vector<std::string>>();
        t.output_name = j.at("output_name").get<std::string>();

        // Estos campos ahora son OPCIONALES:
        t.includes = j.value("includes", std::vector<std::string>{});
        t.system_libs = j.value("system_libs", std::vector<std::string>{});
        t.dependencies = j.value("dependencies", std::vector<std::string>{});
        t.build_dir = j.value("build_dir", std::string{"build"});
        if (t.type == "executable")
            t.output_dir = j.value("output_dir", std::string{"bin"});
        else if (t.type == "shared_lib")
            t.output_dir = j.value("output_dir", std::string{"lib"});
        t.cxxflags = j.value("cxxflags", std::vector<std::string>{});
        t.ldflags = j.value("ldflags", std::vector<std::string>{});
    }
} // namespace KasProjectConfig

#endif // KASPROJECTCONFIG_HPP