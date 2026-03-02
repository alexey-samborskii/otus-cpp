#include "ip_filter.hpp"
#include <iostream>

int main(int, const char*[])
{
    try
    {
        IpStorage ip_storage;

        for (std::string line; std::getline(std::cin, line);)
        {
            auto pos    = line.find('\t');
            auto ip_str = (pos == std::string::npos) ?
                              std::string_view(line) :
                              std::string_view(line).substr(0, pos);
            ip_storage.emplace(ip_str);
        }

        for (const auto& ip : ip_storage.ip_pool())
            std::cout << ip << '\n';

        for (const auto& ip : ip_storage.filter_ip(1))
            std::cout << ip << '\n';

        for (const auto& ip : ip_storage.filter_ip(46, 70))
             std::cout << ip << '\n';

        for (const auto& ip : ip_storage.filter_any(46))
             std::cout << ip << '\n';
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
