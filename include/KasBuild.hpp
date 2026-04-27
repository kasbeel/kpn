#ifndef KASBUILD_HPP
#define KASBUILD_HPP
#include "KasConcurrency.hpp"
#include "KasEngine.hpp"
#include "KasException.hpp"
#include "KasFileSystem.hpp"
#include "KasUtils.hpp"

#include "KasLog.hpp"
#include "KasProjectConfig.hpp"

#include <set>
#include <vector>

namespace KasBuild {
    bool build_target(const KasProjectConfig::KasProject &project, std::string target_req);
} // namespace KasBuild
#endif