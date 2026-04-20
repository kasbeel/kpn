#include <iostream>
#include <cstdarg>
#include <unistd.h>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <filesystem>

struct BuildSettings
{
    std::string project_name;
    std::string compiler = "g++";
    std::string cpp_std = "c++17";
    std::string out_dir = "bin";
    std::string src_ext = "cpp";
    std::vector<std::string> src_dir = {"src"};
    std::vector<std::string> inc_dir = {"include"};
    std::string inc_ext = "hpp";
    std::vector<std::string> flags;
    std::vector<std::string> system_libs;
};

namespace KasLog
{
    enum class Level
    {
        DEBUG,
        INFO,
        WARN,
        ERR
    };

    void log(Level level, const char *fmt, ...)
    {
        bool is_terminal = isatty(STDOUT_FILENO);

        const char *label = "INFO";
        const char *color = "";
        const char *reset = "";

        if (is_terminal)
        {
            reset = "\033[0m";
            switch (level)
            {
            case Level::DEBUG:
                color = "\033[36m";
                label = "DEBUG";
                break;
            case Level::INFO:
                color = "\033[32m";
                label = "INFO ";
                break;
            case Level::WARN:
                color = "\033[33m";
                label = "WARN ";
                break;
            case Level::ERR:
                color = "\033[31m";
                label = "ERROR";
                break;
            }
        }
        else
        {
            switch (level)
            {
            case Level::DEBUG:
                label = "DEBUG";
                break;
            case Level::INFO:
                label = "INFO";
                break;
            case Level::WARN:
                label = "WARN";
                break;
            case Level::ERR:
                label = "ERROR";
                break;
            }
        }

        char message[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(message, sizeof(message), fmt, args);
        va_end(args);
        printf("%s[%s]%s %s\n", color, label, reset, message);
    }
}

namespace KasJson
{
    // Función auxiliar para extraer valores simples de un JSON manual
    std::string get_json_value(const std::string &content, std::string key)
    {
        size_t pos = content.find("\"" + key + "\"");
        if (pos == std::string::npos)
            return "";

        size_t start = content.find(":", pos);
        start = content.find("\"", start) + 1;
        size_t end = content.find("\"", start);
        return content.substr(start, end - start);
    }

    // Función para extraer listas (como flags o libs)
    std::vector<std::string> get_json_array(const std::string &content, std::string key)
    {
        std::vector<std::string> result;
        size_t pos = content.find("\"" + key + "\"");
        if (pos == std::string::npos)
            return result;

        size_t start = content.find("[", pos);
        size_t end = content.find("]", start);
        std::string array_content = content.substr(start + 1, end - start - 1);

        std::stringstream ss(array_content);
        std::string item;
        while (std::getline(ss, item, ','))
        {
            // Limpiar comillas y espacios
            item.erase(std::remove(item.begin(), item.end(), '\"'), item.end());
            item.erase(std::remove(item.begin(), item.end(), ' '), item.end());
            item.erase(std::remove(item.begin(), item.end(), '\n'), item.end());
            if (!item.empty())
                result.push_back(item);
        }
        return result;
    }
}

namespace KasBuild
{
    std::vector<std::string> resolve_pattern(const std::string &pattern, const std::string &ext, bool only_dirs = false)
    {
        std::vector<std::string> results;
        std::string target_ext = "." + ext;

        if (pattern.size() > 3 && pattern.substr(pattern.size() - 3) == "/**")
        {
            std::string base_dir = pattern.substr(0, pattern.size() - 3);
            if (!std::filesystem::exists(base_dir))
                return results;

            if (only_dirs)
                results.push_back(base_dir);

            for (const auto &entry : std::filesystem::recursive_directory_iterator(base_dir))
            {
                if (only_dirs)
                {
                    if (entry.is_directory())
                        results.push_back(entry.path().string());
                }
                else
                {
                    if (entry.is_regular_file() && entry.path().extension() == target_ext)
                    {
                        results.push_back(entry.path().string());
                    }
                }
            }
        }
        else if (pattern.size() > 2 && pattern.substr(pattern.size() - 2) == "/*")
        {
            std::string base_dir = pattern.substr(0, pattern.size() - 2);
            if (!std::filesystem::exists(base_dir))
                return results;

            if (only_dirs)
                results.push_back(base_dir);

            for (const auto &entry : std::filesystem::directory_iterator(base_dir))
            {
                if (only_dirs)
                {
                    if (entry.is_directory())
                        results.push_back(entry.path().string());
                }
                else
                {
                    if (entry.is_regular_file() && entry.path().extension() == target_ext)
                    {
                        results.push_back(entry.path().string());
                    }
                }
            }
        }
        else
        {
            if (std::filesystem::exists(pattern))
            {
                if (only_dirs && std::filesystem::is_directory(pattern))
                {
                    results.push_back(pattern);
                }
                else if (!only_dirs && std::filesystem::is_regular_file(pattern))
                {
                    results.push_back(pattern);
                }
            }
            else
            {
                KasLog::log(KasLog::Level::ERR, "Path not found: %s", pattern.c_str());
            }
        }

        return results;
    }
    std::string exec_cmd(const char *cmd)
    {
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

    void run_build(const BuildSettings &settings)
    {
        KasLog::log(KasLog::Level::INFO, "Building project: %s", settings.project_name.c_str());

        // 1. Preparar Flags de Sistema
        std::string lib_flags = "";
        for (const auto &lib : settings.system_libs)
        {
            std::string check = exec_cmd(("pkg-config --exists " + lib + " && echo 1").c_str());
            if (check != "1")
            {
                KasLog::log(KasLog::Level::ERR, "Missing system dependency: %s", lib.c_str());
                KasLog::log(KasLog::Level::INFO, "Try: sudo apt install lib%s-dev", lib.c_str());
                return;
            }
            lib_flags += exec_cmd(("pkg-config --cflags --libs " + lib).c_str()) + " ";
        }

        // 2. Flags de Usuario
        std::string user_flags = "";
        for (const auto &f : settings.flags)
            user_flags += f + " ";

        // 3. Buscar fuentes (Recursivo simple)
        std::string sources = exec_cmd(("find " + settings.src_dir + " -name '*.cpp' | tr '\\n' ' '").c_str());

        // 4. Comando Final
        std::string command = settings.compiler + " -std=" + settings.cpp_std + " " +
                              user_flags + " " + sources + " -o " + settings.out_dir + "/" + settings.project_name + " " + lib_flags;

        KasLog::log(KasLog::Level::DEBUG, "Command: %s", command.c_str());

        // Crear directorio de salida
        system(("mkdir -p " + settings.out_dir).c_str());

        int res = system(command.c_str());
        if (res == 0)
        {
            KasLog::log(KasLog::Level::INFO, "Build Successful!");
        }
        else
        {
            KasLog::log(KasLog::Level::ERR, "Build Failed with code %d", res);
        }
    }
}

int main(int argc, char *argv[])
{
    // Leer archivo
    std::ifstream file("kas_package.json");
    if (!file.is_open())
    {
        KasLog::log(KasLog::Level::ERR, "Could not find kas_package.json");
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // Cargar settings
    BuildSettings settings;
    settings.project_name = KasJson::get_json_value(content, "project");
    settings.compiler = KasJson::get_json_value(content, "compiler");
    settings.cpp_std = KasJson::get_json_value(content, "std");
    settings.src_dir = KasJson::get_json_array(content, "sources");
    settings.inc_dir = KasJson::get_json_array(content, "includes");
    settings.out_dir = KasJson::get_json_value(content, "out_dir");
    settings.flags = KasJson::get_json_array(content, "flags");
    settings.system_libs = KasJson::get_json_array(content, "system_libs");

    // Ejecutar build si el comando es "build"
    if (argc > 1 && std::string(argv[1]) == "build")
    {
        KasBuild::run_build(settings);
    }
    else
    {
        KasLog::log(KasLog::Level::INFO, "KPN v1.0 - Usage: ./kpn build");
    }

    return 0;
}