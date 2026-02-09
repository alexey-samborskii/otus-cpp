#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <array>
#include <map>
#include <set>
#include <list>

//-----------------------------------------------------------------------------

inline uint32_t ipToKey(uint8_t first, uint8_t second, uint8_t third, uint8_t fourth)
{
    return (static_cast<uint32_t>(first) << 24) |
           (static_cast<uint32_t>(second) << 16) |
           (static_cast<uint32_t>(third) << 8) |
           (static_cast<uint32_t>(fourth));
}

//-----------------------------------------------------------------------------

inline uint32_t ipToKey(const std::vector<std::string> &ip)
{
    if (ip.size() != 4)
    {
        throw std::invalid_argument("IPv4 must have 4 octets");
    }

    auto octet = [](const std::string &s) -> uint32_t {
        std::size_t   pos = 0;
        unsigned long v   = std::stoul(s, &pos, 10);
        if (pos != s.size() || v > 255)
        {
            throw std::invalid_argument("Invalid IPv4 octet: " + s);
        }
        return static_cast<uint8_t>(v);
    };

    return ipToKey(octet(ip[0]), octet(ip[1]), octet(ip[2]), octet(ip[3]));
}

//-----------------------------------------------------------------------------

auto keyToIp(uint32_t key_ip) -> std::array<std::uint8_t, 4>
{
    return {
        uint8_t((key_ip >> 24) & 0xFF),
        uint8_t((key_ip >> 16) & 0xFF),
        uint8_t((key_ip >> 8) & 0xFF),
        uint8_t((key_ip) & 0xFF),
    };
};

//-----------------------------------------------------------------------------

inline std::vector<std::string> split(const std::string &str, char d)
{
    std::vector<std::string> r;

    std::string::size_type start = 0;
    std::string::size_type stop  = str.find_first_of(d);
    while (stop != std::string::npos)
    {
        r.push_back(str.substr(start, stop - start));

        start = stop + 1;
        stop  = str.find_first_of(d, start);
    }

    r.push_back(str.substr(start));

    return r;
}

//-----------------------------------------------------------------------------

inline auto range(std::multiset<uint32_t>::const_iterator begin,
                  std::multiset<uint32_t>::const_iterator end) -> std::list<uint32_t>
{
    std::list<uint32_t> result;
    for (auto it = begin; it != end; ++it)
    {
        result.push_back(*it);
    }
    return result;
}

//-----------------------------------------------------------------------------

inline auto filter(const std::multiset<uint32_t> &ip_pool, uint8_t first) -> std::list<uint32_t>
{
    auto begin = ip_pool.lower_bound(ipToKey(first, 0, 0, 0));
    auto end   = ip_pool.upper_bound(ipToKey(first, 255, 255, 255));
    return range(begin, end);
}

//-----------------------------------------------------------------------------

inline auto filter(const std::multiset<uint32_t> &ip_pool, uint8_t first, uint8_t second) -> std::list<uint32_t>
{
    auto begin = ip_pool.lower_bound(ipToKey(first, second, 0, 0));
    auto end   = ip_pool.upper_bound(ipToKey(first, second, 255, 255));
    return range(begin, end);
}

//-----------------------------------------------------------------------------

inline auto filter_any(const std::multiset<uint32_t> &ip_pool, uint8_t any) -> std::list<uint32_t>
{
    std::list<uint32_t> result;
    for (auto it = ip_pool.begin(); it != ip_pool.end(); ++it)
    {
        const auto &[first, second, third, fourth] = keyToIp(*it);
        if (first == any || second == any || third == any || fourth == any)
        {
            result.push_back(*it);
        }
    }
    return result;
}

//-----------------------------------------------------------------------------

inline void print(const std::list<uint32_t> &ip)
{
    for (auto it = ip.rbegin(); it != ip.rend(); ++it)
    {
        const auto &[first, second, third, fourth] = keyToIp(*it);
        std::cout << +first << "." << +second << "." << +third << "." << +fourth << '\n';
    }
}

//-----------------------------------------------------------------------------

int main(int, const char *[])
{
    try
    {
        std::multiset<uint32_t> ip_pool;

        for (std::string line; std::getline(std::cin, line);)
        {
            auto v = split(line, '\t');
            ip_pool.emplace(ipToKey(split(v.at(0), '.')));
        }

        // 1) Полный список адресов после сортировки. Одна строка - один адрес.
        auto ip = range(ip_pool.begin(), ip_pool.end());
        print(ip);

        // 2) Cписок адресов, первый байт которых равен 1.
        ip = filter(ip_pool, 1);
        print(ip);

        // 3) Cписок адресов, первый байт которых равен 46, а второй 70.
        ip = filter(ip_pool, 46, 70);
        print(ip);

        // 4) Cписок адресов, любой байт которых равен 46.
        ip = filter_any(ip_pool, 46);
        print(ip);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

//-----------------------------------------------------------------------------
