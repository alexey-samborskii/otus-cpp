#include <gtest/gtest.h>
#include <map>

#include "allocator.h"
#include "container.h"

int factorial(int n)
{
    int r = 1;
    for (int i = 1; i <= n; i++)
        r *= i;
    return r;
}

//------------------------------------------------------------------------------

TEST(CustomAllocatorTest, AllocateMultiple)
{
    CustomAllocator<int, 10> alloc;

    int* p1 = alloc.allocate(1);
    int* p2 = alloc.allocate(1);

    ASSERT_NE(p1, p2);

    *p1 = 1;
    *p2 = 2;

    ASSERT_EQ(*p1, 1);
    ASSERT_EQ(*p2, 2);
}

//------------------------------------------------------------------------------

TEST(CustomAllocatorTest, Overflow)
{
    CustomAllocator<int, 3> alloc;

    alloc.allocate(1);
    alloc.allocate(1);
    alloc.allocate(1);

    ASSERT_THROW(
        alloc.allocate(1),
        std::bad_alloc);
}

//------------------------------------------------------------------------------

TEST(MapTest, DefaultAllocator)
{
    std::map<int, int> m;

    for (int i = 0; i < 10; i++)
        m[i] = factorial(i);

    ASSERT_EQ(m[0], 1);
    ASSERT_EQ(m[5], 120);
    ASSERT_EQ(m[9], 362880);
}

//------------------------------------------------------------------------------

TEST(MapTest, CustomAllocator)
{
    std::map<
        int,
        int,
        std::less<int>,
        CustomAllocator<std::pair<const int, int>, 10>>
        m;

    for (int i = 0; i < 10; i++)
        m[i] = factorial(i);

    ASSERT_EQ(m.size(), 10);

    ASSERT_EQ(m[3], 6);
    ASSERT_EQ(m[4], 24);
}

//------------------------------------------------------------------------------

TEST(ContainerTest, DefaultAllocator)
{
    MyContainer<int> c;

    for (int i = 0; i < 10; i++)
        c.push(i);

    int expected = 9;

    for (auto v : c)
    {
        ASSERT_EQ(v, expected);
        expected--;
    }
}

//------------------------------------------------------------------------------

TEST(ContainerTest, CustomAllocator)
{
    MyContainer<int, CustomAllocator<int, 10>> c;

    for (int i = 0; i < 10; i++)
        c.push(i);

    int expected = 9;

    for (auto v : c)
    {
        ASSERT_EQ(v, expected);
        expected--;
    }
}

//------------------------------------------------------------------------------

TEST(ContainerTest, Overflow)
{
    MyContainer<int, CustomAllocator<int, 5>> c;

    for (int i = 0; i < 5; i++)
        c.push(i);

    ASSERT_THROW(
        c.push(10),
        std::bad_alloc);
}

//------------------------------------------------------------------------------