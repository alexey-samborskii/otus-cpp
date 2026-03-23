#pragma once

#include <cstddef>
#include <new>
#include <array>
#include <bitset>

template <typename T, std::size_t N>
class CustomAllocator
{
public:
    using value_type = T;

    template <class U>
    struct rebind
    {
        using other = CustomAllocator<U, N>;
    };

    CustomAllocator() noexcept
    {
        elements_free.set();
    }

    template <class U>
    CustomAllocator(const CustomAllocator<U, N>&) noexcept
    {
    }

    T* allocate(std::size_t n)
    {
        if (n == 0)
            throw std::bad_alloc{};

        if (n > N)
            return static_cast<T*>(::operator new(n * sizeof(T)));

        std::size_t consecutive_free = 0;

        for (std::size_t idx = 0; idx < N; ++idx)
        {
            if (elements_free[idx])
            {
                consecutive_free++;
                
                if (consecutive_free == n)
                {
                    std::size_t start = idx - n + 1;

                    for (std::size_t i = 0; i < n; ++i)
                    {
                        elements_free.reset(start + i);
                    }

                    return reinterpret_cast<T*>(&buffer[start].storage);
                }
            }
            else
            {
                consecutive_free = 0;
            }
        }

        throw std::bad_alloc{};
    }

    void deallocate(T* p, std::size_t n) noexcept
    {
        if (!p)
            return;

        auto* begin = reinterpret_cast<std::byte*>(&buffer[0]);
        auto* end   = reinterpret_cast<std::byte*>(&buffer[N]);
        auto* ptr   = reinterpret_cast<std::byte*>(p);

        if (ptr < begin || ptr >= end)
        {
            ::operator delete(p, n * sizeof(T)); 
            return;
        }

        auto offset = ptr - begin;

        if (offset % sizeof(Node) != 0)
            return;

        std::size_t idx = offset / sizeof(Node);

        if (idx + n > N)
            return;

        for (std::size_t i = 0; i < n; ++i)
        {
            elements_free.set(idx + i);
        }
    }

private:
    struct Node
    {
        alignas(T) unsigned char storage[sizeof(T)];
    };

    std::array<Node, N> buffer;
    std::bitset<N>      elements_free;
};