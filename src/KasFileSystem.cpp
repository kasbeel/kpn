#include <KasFileSystem.hpp>

namespace KasFS {
    std::string get_object_path(const std::string &src, const std::string &obj_folder) {
        fs::path p(src);
        p.replace_extension(".o");
        return (fs::path(obj_folder) / p).string();
    }

    std::vector<std::string> resolve_pattern(const std::string &pattern, const std::string &ext, bool only_dirs) {
        std::vector<std::string> results;
        const std::string target_ext = "." + ext;

        try {
            if (pattern.ends_with("/**")) {
                fs::path base = pattern.substr(0, pattern.size() - 3);
                if (!fs::exists(base))
                    return results;

                for (const auto &entry : fs::recursive_directory_iterator(base)) {
                    if (only_dirs && entry.is_directory())
                        results.push_back(entry.path().string());
                    else if (!only_dirs && entry.is_regular_file() && entry.path().extension() == target_ext)
                        results.push_back(entry.path().string());
                }
            } else if (pattern.ends_with("/*")) {
                fs::path base = pattern.substr(0, pattern.size() - 2);
                if (!fs::exists(base))
                    return results;

                for (const auto &entry : fs::directory_iterator(base)) {
                    if (only_dirs && entry.is_directory())
                        results.push_back(entry.path().string());
                    else if (!only_dirs && entry.is_regular_file() && entry.path().extension() == target_ext)
                        results.push_back(entry.path().string());
                }
            } else if (fs::exists(pattern)) {
                results.push_back(pattern);
            }
        } catch (...) {
            throw KasException::KasBMException("Error resolving pattern: " + pattern);
        }

        return results;
    }

    std::vector<std::string> parse_dep_file(const std::string &dep_file) {
        std::vector<std::string> deps;
        std::ifstream file(dep_file);
        if (!file.is_open())
            return deps;

        std::string line;
        bool main_rule = false;
        while (std::getline(file, line)) {
            if (line.empty())
                continue;

            size_t colon = line.find(':');
            if (!main_rule && colon != std::string::npos) {
                std::istringstream iss(line.substr(colon + 1));
                std::string d;
                while (iss >> d)
                    if (d != "\\" && d.find("/usr/") != 0)
                        deps.push_back(d);

                main_rule = true;
                if (line.find('\\') == std::string::npos)
                    break;
            } else if (main_rule) {
                if (colon != std::string::npos && colon == line.find_last_not_of(" \t\n\r\\"))
                    break;

                std::istringstream iss(line);
                std::string d;
                while (iss >> d)
                    if (d != "\\" && d.find("/usr/") != 0)
                        deps.push_back(d);

                if (line.find('\\') == std::string::npos)
                    break;
            }
        }

        return deps;
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
} // namespace KasFS