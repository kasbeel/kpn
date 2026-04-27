#ifndef KASPACKAGEMANGMENT_HPP
#define KASPACKAGEMANGMENT_HPP

namespace KasPackageManagement {
    void install_package(const std::string &package_name);
    void remove_package(const std::string &package_name);
    void create_package_deb();
} // namespace KasPackageManagement

#endif // KASPACKAGEMANGMENT_HPP
