#include <KasProjectConfig.hpp>

namespace KasProjectConfig {
    void from_json(const json &j, WarningsConfig &w) {
        w.level = j.at("level").get<std::string>();
        w.extra = j.at("extra").get<bool>();
        w.pedantic = j.at("pedantic").get<bool>();
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

    void from_json(const json &j, ProfileConfig &p) {
        p.defines = j.at("defines").get<std::vector<std::string>>();
        p.cxxflags = j.at("cxxflags").get<std::vector<std::string>>();
        p.lto = j.at("lto").get<bool>();
        p.strip = j.at("strip").get<bool>();
    }

    void from_json(const json &j, TargetConfig &t) {
        t.name = j.at("name").get<std::string>();
        t.type = j.at("type").get<std::string>();
        t.sources = j.at("sources").get<std::vector<std::string>>();
        t.output_name = j.at("output_name").get<std::string>();

        t.includes = j.value("includes", std::vector<std::string>{});
        t.system_libs = j.value("system_libs", std::vector<std::string>{});
        t.dependencies = j.value("dependencies", std::vector<std::string>{});
        t.build_dir = j.value("build_dir", std::string{"build"});

        if (t.type == "executable") {
            t.output_dir = j.value("output_dir", std::string{"bin"});
            t.output_extension = j.value("output_extension", std::string{});
        } else if (t.type == "shared_lib") {
            t.output_dir = j.value("output_dir", std::string{"lib"});
            t.output_extension = j.value("output_extension", std::string{".so"});
        } else if (t.type == "static_lib") {
            t.output_dir = j.value("output_dir", std::string{"lib"});
            t.output_extension = j.value("output_extension", std::string{".a"});
        }

        t.cxxflags = j.value("cxxflags", std::vector<std::string>{});
        t.ldflags = j.value("ldflags", std::vector<std::string>{});
    }

    void from_json(const json &j, CommandConfig &c) {
        c.enabled = j.at("enabled").get<bool>();
        c.command = j.at("command").get<std::string>();
    }

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
} // namespace KasProjectConfig