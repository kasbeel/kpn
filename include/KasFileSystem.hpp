#ifndef KASFILESYSTEM_HPP
#define KASFILESYSTEM_HPP
#include "KasException.hpp"
#include "KasLog.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace KasFS {
    std::string get_object_path(const std::string &src, const std::string &obj_folder);

    std::vector<std::string> resolve_pattern(const std::string &pattern, const std::string &ext, bool only_dirs = false);

    std::vector<std::string> parse_dep_file(const std::string &dep_file);
    void update_symlink(const std::filesystem::path &target, const std::filesystem::path &link_name);
} // namespace KasFS
#endif // KASFILESYSTEM_HPP