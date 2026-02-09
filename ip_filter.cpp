#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <array>
#include <map>
#include <set>

//-----------------------------------------------------------------------------

uint32_t ipToKey(const std::vector<std::string> &ip)
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
        return static_cast<uint32_t>(v);
    };

    return (octet(ip[0]) << 24) |
           (octet(ip[1]) << 16) |
           (octet(ip[2]) << 8) |
           (octet(ip[3]));
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

// ("",  '.') -> [""]
// ("11", '.') -> ["11"]
// ("..", '.') -> ["", "", ""]
// ("11.", '.') -> ["11", ""]
// (".11", '.') -> ["", "11"]
// ("11.22", '.') -> ["11", "22"]

//-----------------------------------------------------------------------------

std::vector<std::string> split(const std::string &str, char d)
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

int main(int, const char *[])
{
    try
    {
        std::set<uint32_t> ip_pool;

        for (std::string line; std::getline(std::cin, line);)
        {
            std::vector<std::string> v = split(line, '\t');
            ip_pool.emplace(ipToKey(split(v.at(0), '.')));
        }

        /*
         * 1.   Полный список адресов после сортировки. Одна строка - один адрес.
         */

        auto begin = --ip_pool.begin();
        auto end   = --ip_pool.end();
        for (auto it = end; it != begin; --it)
        {
            const auto &[first, second, third, fourth] = keyToIp(*it);
            std::cout << +first << "." << +second << "." << +third << "." << +fourth << '\n';
        }

        /*
         * 2.   Сразу следом список адресов, первый байт которых равен 1.
         *      Порядок сортировки не меняется.
         *      Одна строка - один адрес. Списки ничем не разделяются.
         */

        begin = --ip_pool.lower_bound(ipToKey({"1", "0", "0", "0"}));
        end   = --ip_pool.upper_bound(ipToKey({"1", "255", "255", "255"}));
        for (auto it = end; it != begin; --it)
        {
            const auto &[first, second, third, fourth] = keyToIp(*it);
            std::cout << +first << "." << +second << "." << +third << "." << +fourth << '\n';
        }

        /*
         * 3.   Сразу продолжается список адресов, первый байт которых равен 46, а второй 70.
         *      Порядок сортировки не меняется.
         *      Одна строка - один адрес. Списки ничем не разделяются.
         */

        begin = --ip_pool.lower_bound(ipToKey({"46", "70", "0", "0"}));
        end   = --ip_pool.upper_bound(ipToKey({"46", "70", "255", "255"}));
        for (auto it = end; it != begin; --it)
        {
            const auto &[first, second, third, fourth] = keyToIp(*it);
            std::cout << +first << "." << +second << "." << +third << "." << +fourth << '\n';
        }

        /*
         * 4.   Сразу продолжается список адресов, любой байт которых равен 46.
         *      Порядок сортировки не меняется.
         *      Одна строка - один адрес.
         *      Списки ничем не разделяются.
         */

        for (auto it = ip_pool.rbegin(); it != ip_pool.rend(); ++it)
        {
            const auto &[first, second, third, fourth] = keyToIp(*it);
            if (first == 46 || second == 46 || third == 46 || fourth == 46)
            {
                std::cout << +first << "." << +second << "." << +third << "." << +fourth << '\n';
            }
        }

        // 222.173.235.246
        // 222.130.177.64
        // 222.82.198.61
        // ...
        // 1.70.44.170
        // 1.29.168.152
        // 1.1.234.8

        // TODO filter by first byte and output
        // ip = filter(1)

        // 1.231.69.33
        // 1.87.203.225
        // 1.70.44.170
        // 1.29.168.152
        // 1.1.234.8

        // TODO filter by first and second bytes and output
        // ip = filter(46, 70)

        // 46.70.225.39
        // 46.70.147.26
        // 46.70.113.73
        // 46.70.29.76

        // TODO filter by any byte and output
        // ip = filter_any(46)

        // 186.204.34.46
        // 186.46.222.194
        // 185.46.87.231
        // 185.46.86.132
        // 185.46.86.131
        // 185.46.86.131
        // 185.46.86.22
        // 185.46.85.204
        // 185.46.85.78
        // 68.46.218.208
        // 46.251.197.23
        // 46.223.254.56
        // 46.223.254.56
        // 46.182.19.219
        // 46.161.63.66
        // 46.161.61.51
        // 46.161.60.92
        // 46.161.60.35
        // 46.161.58.202
        // 46.161.56.241
        // 46.161.56.203
        // 46.161.56.174
        // 46.161.56.106
        // 46.161.56.106
        // 46.101.163.119
        // 46.101.127.145
        // 46.70.225.39
        // 46.70.147.26
        // 46.70.113.73
        // 46.70.29.76
        // 46.55.46.98
        // 46.49.43.85
        // 39.46.86.85
        // 5.189.203.46
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}

//-----------------------------------------------------------------------------
