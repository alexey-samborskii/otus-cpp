#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <array>
#include <stdexcept>
#include <charconv>
#include <set>
#include <list>
#include <ostream>

//------------------------------------------------------------------------------

class IpAddress
{
public:
    constexpr IpAddress(uint8_t first,
                        uint8_t second,
                        uint8_t third,
                        uint8_t fourth)
        : key_ip_(toKey(first, second, third, fourth))
    {
    }

    explicit IpAddress(std::string_view ipv4)
    {
        std::array<std::string_view, 4> parts{};
        size_t                          part_index = 0;
        size_t                          start      = 0;

        for (size_t i = 0; i < ipv4.size(); ++i)
        {
            if (ipv4[i] == '.')
            {
                if (part_index >= 3)
                    throw std::invalid_argument("Invalid IPv4 format");

                parts[part_index++] = ipv4.substr(start, i - start);
                start               = i + 1;
            }
        }

        parts[part_index++] = ipv4.substr(start);

        if (part_index != 4)
            throw std::invalid_argument("IPv4 must have 4 octets");

        key_ip_ = toKey(parseOctet(parts[0]),
                        parseOctet(parts[1]),
                        parseOctet(parts[2]),
                        parseOctet(parts[3]));
    }

    [[nodiscard]] uint32_t key() const noexcept
    {
        return key_ip_;
    }

    [[nodiscard]] std::array<uint8_t, 4> ip() const noexcept
    {
        return {
            static_cast<uint8_t>((key_ip_ >> 24) & 0xFF),
            static_cast<uint8_t>((key_ip_ >> 16) & 0xFF),
            static_cast<uint8_t>((key_ip_ >> 8) & 0xFF),
            static_cast<uint8_t>((key_ip_) & 0xFF),
        };
    }

    bool operator<(const IpAddress& other) const noexcept
    {
        return key_ip_ > other.key_ip_;
    }

    bool operator==(const IpAddress& other) const noexcept
    {
        return key_ip_ == other.key_ip_;
    }

private:
    static constexpr uint32_t toKey(uint8_t first,
                                    uint8_t second,
                                    uint8_t third,
                                    uint8_t fourth) noexcept
    {
        return ((static_cast<uint32_t>(first) << 24) |
                (static_cast<uint32_t>(second) << 16) |
                (static_cast<uint32_t>(third) << 8) |
                (static_cast<uint32_t>(fourth)));
    }

    static uint8_t parseOctet(std::string_view str)
    {
        unsigned int value = 0;

        auto [ptr, ec] = std::from_chars(str.data(),
                                         str.data() + str.size(),
                                         value);

        if (ec != std::errc() || ptr != str.data() + str.size() || value > 255)
            throw std::invalid_argument("Invalid IPv4 octet");

        return static_cast<uint8_t>(value);
    }

private:
    uint32_t key_ip_{};
};

//------------------------------------------------------------------------------

inline std::ostream& operator<<(std::ostream& os, const IpAddress& ip)
{
    auto ip_array = ip.ip();

    return os << static_cast<unsigned>(ip_array[0]) << '.'
              << static_cast<unsigned>(ip_array[1]) << '.'
              << static_cast<unsigned>(ip_array[2]) << '.'
              << static_cast<unsigned>(ip_array[3]);
}

//------------------------------------------------------------------------------

class IpStorage
{
public:
    void emplace(std::string_view str)
    {
        ip_pool_.emplace(str);
    }

    [[nodiscard]] const std::multiset<IpAddress>& ip_pool() const noexcept
    {
        return ip_pool_;
    }

    template <typename... Octets>
    auto filter_ip(Octets... octets) const -> std::list<IpAddress>
    {
        static_assert(sizeof...(octets) >= 1 && sizeof...(octets) <= 4,
                      "IPv4 filter supports 1 to 4 octets");

        static_assert((std::is_integral_v<Octets> && ...),
                      "All octets must be integral types");

        if (((octets > 255) || ...))
            throw std::invalid_argument("Octet must be <= 255");

        constexpr size_t count = sizeof...(octets);

        std::array<uint8_t, 4> low{0, 0, 0, 0};
        std::array<uint8_t, 4> high{255, 255, 255, 255};

        std::array<uint8_t, count> input{static_cast<uint8_t>(octets)...};

        for (size_t i = 0; i < count; ++i)
        {
            low[i]  = input[i];
            high[i] = input[i];
        }

        return filter(IpAddress{high[0], high[1], high[2], high[3]},
                      IpAddress{low[0], low[1], low[2], low[3]});
    }

    auto filter_any(uint8_t any) const -> std::list<IpAddress>
    {
        std::list<IpAddress> result;
        for (auto it = ip_pool_.begin(); it != ip_pool_.end(); ++it)
        {
            const auto& [first, second, third, fourth] = it->ip();
            if (first == any || second == any || third == any || fourth == any)
            {
                result.push_back(*it);
            }
        }
        return result;
    }

private:
    auto filter(IpAddress ip_begin, IpAddress ip_end) const -> std::list<IpAddress>
    {
        auto begin = ip_pool_.lower_bound(ip_begin);
        auto end   = ip_pool_.upper_bound(ip_end);

        std::list<IpAddress> result;
        for (auto it = begin; it != end; ++it)
        {
            result.push_back(*it);
        }
        return result;
    }

private:
    std::multiset<IpAddress> ip_pool_;
};

//------------------------------------------------------------------------------