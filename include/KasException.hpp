#ifndef KASEXCEPTION_HPP
#define KASEXCEPTION_HPP
#include <stdexcept>
#include <string>

namespace KasException {
    class KasBMException : public std::runtime_error {
      public:
        explicit KasBMException(const std::string &message);
    };
} // namespace KasException
#endif // KASEXCEPTION_HPP