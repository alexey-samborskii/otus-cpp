#pragma once

#include <cstddef>
#include <memory>
#include <new>

template <typename T, std::size_t N>
class CustomAllocator
{
public:
    using value_type = T;
    using size_type  = std::size_t;
    using pointer    = T *;

    CustomAllocator() noexcept
    {
    }

    template <typename U>
    CustomAllocator(const CustomAllocator<U, N> &) noexcept
    {
    }

    pointer allocate(size_type n)
    {
        if (offset + n > N)
            throw std::bad_alloc();

        pointer ptr =
            reinterpret_cast<pointer>(buffer + offset * sizeof(T));

        offset += n;

        return ptr;
    }

    void deallocate(pointer, size_type) noexcept
    {
        // память освобождается целиком при уничтожении allocator
    }

    template <typename U>
    struct rebind
    {
        using other = CustomAllocator<U, N>;
    };

private:
    alignas(T) char buffer[sizeof(T) * N];
    size_type offset = 0;
};