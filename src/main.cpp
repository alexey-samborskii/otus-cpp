#include <iostream>
#include <map>

#include "allocator.h"
#include "container.h"

//------------------------------------------------------------------------------

int factorial(int n)
{
    int r = 1;

    for (int i = 1; i <= n; i++)
        r *= i;

    return r;
}

//------------------------------------------------------------------------------

int main()
{
    std::cout << "std::map default allocator\n";

    std::map<int, int> m;

    for (int i = 0; i < 10; i++)
        m[i] = factorial(i);

    for (auto &v : m)
        std::cout << v.first << " " << v.second << std::endl;

    std::cout << "\nstd::map custom allocator\n";

    std::map<int,
             int,
             std::less<int>,
             CustomAllocator<std::pair<const int, int>, 10>>
        m2;

    for (int i = 0; i < 10; i++)
        m2[i] = factorial(i);

    for (auto &v : m2)
        std::cout << v.first << " " << v.second << std::endl;

    std::cout << "\nMyContainer default allocator\n";

    MyContainer<int> c;

    for (int i = 0; i < 10; i++)
        c.push(i);

    for (auto v : c)
        std::cout << v << std::endl;

    std::cout << "\nMyContainer custom allocator\n";

    MyContainer<int, CustomAllocator<int, 10>> c2;

    for (int i = 0; i < 10; i++)
        c2.push(i);

    for (auto v : c2)
        std::cout << v << std::endl;
}

//------------------------------------------------------------------------------