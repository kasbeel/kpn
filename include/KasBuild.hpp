#ifndef KASBUILD_HPP
#define KASBUILD_HPP
#include "KasLog.hpp"
#include "KasProjectConfig.hpp"
#include <algorithm>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <future>
#include <mutex>
#include <set>
#include <sstream>
#include <thread>
#include <vector>

namespace KasBuild {
    class Semaphore {
      private:
        std::mutex mtx;
        std::condition_variable cv;
        int count;

      public:
        Semaphore(int n) : count(n) {}

        void wait() {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this]() { return count > 0; });
            count--;
        }

        void notify() {
            std::lock_guard<std::mutex> lock(mtx);
            count++;
            cv.notify_one();
        }
    };
    bool check_system_dependency(const std::string &lib_name) {
        std::string command = "pkg-config --exists " + lib_name + " 2>/dev/null";

        int result = std::system(command.c_str());
        KasLog::log(KasLog::Level::DEBUG, "Checking system dependency: %s %s", lib_name.c_str(), (result == 0) ? "[OK]" : "[MISSING]");
        if (result == 0) {
            return true;
        } else {
            KasLog::log(KasLog::Level::ERR, "Missing system library: %s", lib_name.c_str());
            KasLog::log(KasLog::Level::INFO, "Suggestion: sudo apt install lib%s", (lib_name + "-dev").c_str());
            return false;
        }
    }
    std::string exec_cmd(const char *cmd) {
        char buffer[128];
        std::string result = "";
        FILE *pipe = popen(cmd, "r");
        if (!pipe)
            return "ERROR";
        while (fgets(buffer, sizeof buffer, pipe) != NULL)
            result += buffer;
        pclose(pipe);
        if (!result.empty() && result.back() == '\n')
            result.pop_back();
        return result;
    }
    const KasProjectConfig::TargetConfig *find_target(const KasProjectConfig::KasProject &project, const std::string &name) {
        for (const auto &t : project.targets) {
            if (t.name == name)
                return &t;
        }
        return nullptr;
    }

    void resolve_order(const std::string &target_name,
                       const KasProjectConfig::KasProject &project,
                       std::vector<std::string> &ordered_list,
                       std::set<std::string> &visited) {

        if (visited.count(target_name))
            return;

        const auto *target = find_target(project, target_name);
        if (!target)
            return;

        for (const auto &dep : target->dependencies) {
            resolve_order(dep, project, ordered_list, visited);
        }

        visited.insert(target_name);
        ordered_list.push_back(target_name);
    }
    std::string get_object_path(const std::string &source_path, const std::string &obj_folder) {
        std::filesystem::path p(source_path);

        p.replace_extension(".o");

        std::filesystem::path obj_path = std::filesystem::path(obj_folder) / p;

        return obj_path.string();
    }

    std::vector<std::string> resolve_pattern(const std::string &pattern, const std::string &ext, bool only_dirs = false) {
        std::vector<std::string> results;
        std::string target_ext = "." + ext;

        if (pattern.size() > 3 && pattern.substr(pattern.size() - 3) == "/**") {
            std::string base_dir = pattern.substr(0, pattern.size() - 3);
            if (!std::filesystem::exists(base_dir))
                return results;

            if (only_dirs)
                results.push_back(base_dir);

            for (const auto &entry : std::filesystem::recursive_directory_iterator(base_dir)) {
                if (only_dirs) {
                    if (entry.is_directory())
                        results.push_back(entry.path().string());
                } else {
                    if (entry.is_regular_file() && entry.path().extension() == target_ext) {
                        results.push_back(entry.path().string());
                    }
                }
            }
        } else if (pattern.size() > 2 && pattern.substr(pattern.size() - 2) == "/*") {
            std::string base_dir = pattern.substr(0, pattern.size() - 2);
            if (!std::filesystem::exists(base_dir))
                return results;

            if (only_dirs)
                results.push_back(base_dir);

            for (const auto &entry : std::filesystem::directory_iterator(base_dir)) {
                if (only_dirs) {
                    if (entry.is_directory())
                        results.push_back(entry.path().string());
                } else {
                    if (entry.is_regular_file() && entry.path().extension() == target_ext) {
                        results.push_back(entry.path().string());
                    }
                }
            }
        } else {
            if (std::filesystem::exists(pattern)) {
                if (only_dirs && std::filesystem::is_directory(pattern)) {
                    results.push_back(pattern);
                } else if (!only_dirs && std::filesystem::is_regular_file(pattern)) {
                    results.push_back(pattern);
                }
            } else {
                KasLog::log(KasLog::Level::ERR, "Path not found: %s", pattern.c_str());
            }
        }

        return results;
    }

    int run_compile(const std::string &src, const std::string &obj, const std::string &compiler, const std::string &cpp_std, const std::string &include_flags, const std::string &extra_flags) {
        std::filesystem::path parent_dir = std::filesystem::path(obj).parent_path();

        if (!std::filesystem::exists(parent_dir)) {
            std::filesystem::create_directories(parent_dir);
        }

        std::string command = compiler + " -std=" + cpp_std + " -c " + src + " -o " + obj + " " + include_flags + " " + extra_flags;
        KasLog::log(KasLog::Level::DEBUG, "%s", command.c_str());
        int result = system(command.c_str());

        if (result == 0) {
            KasLog::log(KasLog::Level::INFO, "%s build successful!", src.c_str());
        } else {
            int exit_code = WEXITSTATUS(result);
            KasLog::log(KasLog::Level::ERR, "%s build failed with code %d", src.c_str(), exit_code);
            exit(exit_code);
        }
        return WEXITSTATUS(result);
    }
    void update_symlink(const std::filesystem::path &target, const std::filesystem::path &link_name) {
        try {
            if (std::filesystem::exists(link_name) || std::filesystem::is_symlink(link_name)) {
                std::filesystem::remove(link_name);
            }
            std::filesystem::create_symlink(target, link_name);

            KasLog::log(KasLog::Level::INFO, "Symlink actualizado: %s -> %s", link_name.c_str(), target.c_str());
        } catch (const std::filesystem::filesystem_error &e) {
            KasLog::log(KasLog::Level::ERR, "No se pudo crear el symlink: %s", e.what());
        }
    }
    int run_linker(const KasProjectConfig::KasProject &project, const KasProjectConfig::TargetConfig &target, const std::string &objects, const std::string &compiler, const std::string &libs_flags) {
        std::filesystem::path output_path = std::filesystem::path(target.output_dir) / target.output_name;

        if (!std::filesystem::exists(target.output_dir)) {
            std::filesystem::create_directories(target.output_dir);
        }

        std::string ld_flags = " ";
        for (const auto &f : project.toolchain.ldflags)
            ld_flags += f + " ";
        for (const auto &f : target.ldflags)
            ld_flags += f + " ";

        std::string output_name = target.output_name;
        if (target.type == "shared_lib") {
            output_name = "lib" + target.output_name + target.output_extension + "." + project.version;
        }

        std::string link_cmd = compiler + ld_flags + objects + " -o " + target.output_dir + "/";
        if (target.type == "shared_lib") {
            link_cmd += output_name + " -shared ";
        } else {
            link_cmd += output_name + " ";
        }
        link_cmd += libs_flags;

        KasLog::log(KasLog::Level::INFO, "Linking %s...", target.name.c_str());
        KasLog::log(KasLog::Level::DEBUG, "%s", link_cmd.c_str());
        if (std::system(link_cmd.c_str()) != 0) {
            KasLog::log(KasLog::Level::ERR, "Linking failed for target: %s", target.name.c_str());
            return 1;
        }
        if (target.type == "shared_lib")
            update_symlink(output_name, target.output_dir + "/" + "lib" + target.output_name + target.output_extension);

        return 0;
    }
    bool needs_recompile(const std::string &source_p, const std::string &object_p) {
        if (!std::filesystem::exists(object_p))
            return true;

        auto source_time = std::filesystem::last_write_time(source_p);
        auto object_time = std::filesystem::last_write_time(object_p);

        return source_time > object_time;
    }
    std::vector<std::string> parse_dep_file(const std::string &dep_file) {
        std::vector<std::string> dependencies;
        std::ifstream file(dep_file);
        if (!file.is_open()) {
            KasLog::log(KasLog::Level::ERR, "Failed to open dependency file: %s", dep_file.c_str());
            return dependencies;
        }

        std::string line;
        while (std::getline(file, line)) {
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                std::string deps_part = line.substr(colon_pos + 1);
                std::istringstream iss(deps_part);
                std::string dep;
                while (iss >> dep) {
                    dependencies.push_back(dep);
                }
            }
        }
        return dependencies;
    }
    bool should_build_obj(const std::string &src, const std::string &obj) {

        if (needs_recompile(src, obj))
            return true;

        std::filesystem::path p(obj);
        p.replace_extension(".d");
        std::string dep_file = p.string();

        if (std::filesystem::exists(dep_file)) {
            std::vector<std::string> dependencies = parse_dep_file(dep_file);
            for (const auto &dep : dependencies) {
                if (std::filesystem::exists(dep)) {
                    if (std::filesystem::last_write_time(dep) > std::filesystem::last_write_time(obj)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }
    int run_compile_target(const KasProjectConfig::KasProject &project, const KasProjectConfig::TargetConfig &target) {
        bool all_deps_ok = true;
        bool needs_relink = false;
        std::string libs = "";
        std::string libs_flags = "";
        std::string includes = "";

        for (const auto &lib : target.system_libs) {
            if (!check_system_dependency(lib)) {
                all_deps_ok = false;
            }
            libs += lib + " ";
        }
        if (!libs.empty()) {
            libs_flags = exec_cmd(("pkg-config --libs " + libs).c_str());
            includes = exec_cmd(("pkg-config --cflags " + libs).c_str());
        }
        for (const auto &dep : target.dependencies) {
            const auto *dep_target = find_target(project, dep);
            if (!dep_target) {
                KasLog::log(KasLog::Level::ERR, "Dependency target not found: %s", dep.c_str());
                all_deps_ok = false;
            } else {
                if (dep_target->type == "shared_lib") {
                    libs_flags += "-L" + dep_target->output_dir + " -l" + dep_target->output_name + " ";
                }
            }
        }

        if (!all_deps_ok) {
            KasLog::log(KasLog::Level::ERR, "Missing system dependencies. Aborting compilation.");
            return 1;
        }

        std::vector<std::string> sources;
        for (const auto &src_pattern : target.sources) {
            std::vector<std::string> src_files = resolve_pattern(src_pattern, "cpp");
            sources.insert(sources.end(), src_files.begin(), src_files.end());
        }
        for (const auto &inc_pattern : target.includes) {
            std::vector<std::string> inc_dirs = resolve_pattern(inc_pattern, "", true);
            for (const auto &d : inc_dirs) {
                includes += "-I" + d + " ";
            }
        }
        std::string extra_flags = "";
        for (const auto &f : project.toolchain.cxxflags)
            extra_flags += f + " ";
        for (const auto &f : target.cxxflags)
            extra_flags += f + " ";

        if (extra_flags.find("-MMD") == std::string::npos)
            extra_flags += "-MMD ";

        if (extra_flags.find("-MP ") == std::string::npos)
            extra_flags += "-MP ";

        if (target.type == "shared_lib" && extra_flags.find("-fPIC") == std::string::npos)
            extra_flags += "-fPIC ";

        int max_threads = project.toolchain.jobs > 0 ? project.toolchain.jobs : std::thread::hardware_concurrency();
        Semaphore sem(max_threads);
        std::vector<std::future<int>> tasks;

        KasLog::log(KasLog::Level::INFO, "Compiling target %s using %d threads", target.name.c_str(), max_threads);
        std::string objects = "";
        for (const auto &src : sources) {
            auto obj = get_object_path(src, target.build_dir);
            objects += obj + " ";
            if (!should_build_obj(src, obj)) {
                KasLog::log(KasLog::Level::DEBUG, "Up to date: %s", src.c_str());
                continue;
            }
            needs_relink = true;
            sem.wait();
            tasks.push_back(std::async(std::launch::async, [&sem, &project, src, obj, includes, extra_flags]() {
                int res = run_compile(src, obj, project.toolchain.compiler, project.toolchain.standards.at("cxx"), includes, extra_flags);

                sem.notify();
                return res;
            }));
        }

        bool success = true;
        for (auto &t : tasks) {
            if (t.get() != 0)
                success = false;
        }
        if (!success) {
            KasLog::log(KasLog::Level::ERR, "Compilation failed for target: %s", target.name.c_str());
            return 1;
        }

        if (needs_relink) {
            return run_linker(project, target, objects, project.toolchain.compiler, libs_flags);
        }
        return 0;
    }
    bool build_target(const KasProjectConfig::KasProject &project, std::string target_req) {
        std::vector<std::string> build_queue;
        std::set<std::string> visited;

        if (target_req == "all") {
            for (const auto &t : project.targets) {
                resolve_order(t.name, project, build_queue, visited);
            }
        } else {
            if (find_target(project, target_req)) {
                resolve_order(target_req, project, build_queue, visited);
            } else {
                KasLog::log(KasLog::Level::ERR, "Target not found: %s", target_req.c_str());
                return false;
            }
        }

        for (const auto &t_name : build_queue) {
            const auto *target = find_target(project, t_name);
            KasLog::log(KasLog::Level::INFO, "Compiling target: %s", t_name.c_str());

            if (run_compile_target(project, *target) != 0) {
                KasLog::log(KasLog::Level::ERR, "Error compiling target: %s", t_name.c_str());
                return false;
            }
        }

        return true;
    }
} // namespace KasBuild
#endif