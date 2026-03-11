#pragma once

#include <memory>

template <typename T, typename Alloc = std::allocator<T>>
class MyContainer
{
    struct Node
    {
        T     value;
        Node* next;
    };

    using NodeAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;

public:
    MyContainer()
        : head(nullptr)
    {
    }

    void push(const T& value)
    {
        Node* node = alloc.allocate(1);

        std::allocator_traits<NodeAlloc>::construct(alloc, node);

        node->value = value;
        node->next  = head;

        head = node;
    }

    struct iterator
    {
        Node* ptr;

        iterator(Node* p)
            : ptr(p)
        {
        }

        T& operator*()
        {
            return ptr->value;
        }

        iterator& operator++()
        {
            ptr = ptr->next;
            return *this;
        }

        bool operator!=(const iterator& other)
        {
            return ptr != other.ptr;
        }
    };

    iterator begin()
    {
        return iterator(head);
    }

    iterator end()
    {
        return iterator(nullptr);
    }

private:
    Node*     head;
    NodeAlloc alloc;
};