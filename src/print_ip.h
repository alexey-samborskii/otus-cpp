#include <iostream>
#include <type_traits>
#include <vector>
#include <list>
#include <tuple>
#include <string>
#include <string_view>

//
// --- INTEGER TYPE ---
//
//------------------------------------------------------------------------------

template <typename T,
          typename = std::enable_if_t<std::is_integral<T>::value>>
void print_ip(T value)
{
    using Unsigned = std::make_unsigned_t<T>;

    const auto        unsigned_value = static_cast<Unsigned>(value);
    const std::size_t size           = sizeof(T);

    for (std::size_t i = 0; i < size; ++i)
    {
        const auto shift = (size - 1 - i) * 8;
        const auto byte  = (unsigned_value >> shift) & 0xFF;

        std::cout << static_cast<unsigned>(byte);

        if (i + 1 != size)
        {
            std::cout << ".";
        }
    }

    std::cout << std::endl;
}

//
// --- STRING ---
//
//------------------------------------------------------------------------------
template <typename T>
using is_string_like = std::disjunction<
    std::is_same<T, std::string>,
    std::is_same<T, std::string_view>,
    std::is_same<T, const char*>,
    std::is_same<T, char*>,
    std::is_same<T, const char[]>>;

template <typename T,
          typename = std::enable_if_t<is_string_like<std::decay_t<T>>::value>>
void print_ip(const T& value)
{
    std::cout << value << std::endl;
}

//
// --- VECTOR / LIST ---
//
//------------------------------------------------------------------------------

template <typename Container>
void print_container_ip(const Container& container)
{
    auto it = container.begin();

    while (it != container.end())
    {
        std::cout << *it;
        ++it;

        if (it != container.end())
        {
            std::cout << ".";
        }
    }

    std::cout << std::endl;
}

template <typename T, typename Allocator>
void print_ip(const std::vector<T, Allocator>& container)
{
    print_container_ip(container);
}

template <typename T, typename Allocator>
void print_ip(const std::list<T, Allocator>& container)
{
    print_container_ip(container);
}

//
// --- TUPLE ---
//
//------------------------------------------------------------------------------

template <typename T,
          typename... Rest,
          typename = std::enable_if_t<std::conjunction<
              std::is_same<T, Rest>...>::value>>
void print_ip(const std::tuple<T, Rest...>& t)
{
    std::apply(
        [](const auto&... args) {
            std::size_t i = 0;
            ((std::cout << (i++ ? "." : "") << args), ...);
            std::cout << std::endl;
        },
        t);
}

//------------------------------------------------------------------------------