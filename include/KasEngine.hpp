#ifndef KASENGINE_HPP
#define KASENGINE_HPP
#include "KasConcurrency.hpp"
#include "KasFileSystem.hpp"
#include "KasProjectConfig.hpp"
#include "KasUtils.hpp"
#include <future>

namespace KasEngine {
    int run_compile(const std::string &src, const std::string &obj, const KasProjectConfig::KasProject &project, const std::string &inc, const std::string &extra);

    void run_linker(const KasProjectConfig::KasProject &project, const KasProjectConfig::TargetConfig &target, const std::string &objs, const std::string &libs);

} // namespace KasEngine
#endif // KASENGINE_HPP