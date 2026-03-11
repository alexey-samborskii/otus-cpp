#pragma once

#include <cstddef>
#include <new>

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
        for (std::size_t i = 0; i < N; ++i)
        {
            buffer[i].ptr = free_list_;
            free_list_    = &buffer[i];
        }
    }

    template <class U>
    CustomAllocator(const CustomAllocator<U, N>&) noexcept
    {
    }

    T* allocate(std::size_t n)
    {
        if (n == 1 && free_list_ != nullptr)
        {
            Node* node = free_list_;
            free_list_ = node->ptr;
            return reinterpret_cast<T*>(node);
        }

        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    void deallocate(T* p, std::size_t n) noexcept
    {
        Node* begin = buffer;
        Node* end   = buffer + N;
        Node* node  = reinterpret_cast<Node*>(p);

        if (node >= begin && node < end)
        {
            node->ptr  = free_list_;
            free_list_ = node;
        }
        else
        {
            ::operator delete(p);
        }
    }

private:
    union Node
    {
        Node* ptr;
        alignas(T) unsigned char storage[sizeof(T)];
    };

    Node  buffer[N];
    Node* free_list_{nullptr};
};