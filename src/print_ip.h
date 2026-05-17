#include <iostream>
#include <type_traits>
#include <vector>
#include <list>
#include <tuple>
#include <string>

//
// --- INTEGER TYPE ---
//
//------------------------------------------------------------------------------

template <typename T>
typename std::enable_if<std::is_integral<T>::value>::type
print_ip(T value)
{
    const std::size_t size = sizeof(T);

    for (std::size_t i = 0; i < size; ++i)
    {
        std::cout << ((value >> ((size - 1 - i) * 8)) & 0xFF);

        if (i + 1 != size)
            std::cout << ".";
    }

    std::cout << std::endl;
}

//
// --- STRING ---
//
//------------------------------------------------------------------------------

template <typename T>
typename std::enable_if<std::is_same<T, std::string>::value>::type
print_ip(const T& value)
{
    std::cout << value << std::endl;
}

//
// --- VECTOR / LIST ---
//
//------------------------------------------------------------------------------

template <typename T>
struct is_container : std::false_type
{
};

template <typename... Args>
struct is_container<std::vector<Args...>> : std::true_type
{
};

template <typename... Args>
struct is_container<std::list<Args...>> : std::true_type
{
};

template <typename T>
typename std::enable_if<is_container<T>::value>::type
print_ip(const T& container)
{
    auto it = container.begin();

    while (it != container.end())
    {
        std::cout << *it;
        ++it;

        if (it != container.end())
            std::cout << ".";
    }

    std::cout << std::endl;
}

//
// --- TUPLE ---
//
//------------------------------------------------------------------------------

template <typename T>
struct all_same;

template <typename T>
struct all_same<std::tuple<T>> : std::true_type
{
};

template <typename T, typename U, typename... Rest>
struct all_same<std::tuple<T, U, Rest...>>
    : std::integral_constant<bool,
                             std::is_same<T, U>::value &&
                                 all_same<std::tuple<U, Rest...>>::value>
{
};

template <typename T>
typename std::enable_if<
    std::tuple_size<T>::value != 0 && all_same<T>::value>::type
print_ip(const T& t)
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