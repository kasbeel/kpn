#include <KasException.hpp>

namespace KasException {
    KasBMException::KasBMException(const std::string &message)
        : std::runtime_error(message) {}
} // namespace KasException