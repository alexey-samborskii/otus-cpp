#include <gtest/gtest.h>

#include "print_ip.h"

#include <sstream>
#include <vector>
#include <list>
#include <tuple>
#include <string>
#include <cstdint>

//
// Helper: перехват std::cout
//
//------------------------------------------------------------------------------

class CoutCapture
{
public:
    CoutCapture()
    {
        old_buf = std::cout.rdbuf(buffer.rdbuf());
    }

    ~CoutCapture()
    {
        std::cout.rdbuf(old_buf);
    }

    std::string str() const
    {
        return buffer.str();
    }

private:
    std::stringstream buffer;
    std::streambuf* old_buf;
};

//
// --- INTEGER TESTS ---
//
//------------------------------------------------------------------------------

TEST(PrintIpTest, Int8)
{
    CoutCapture cap;
    print_ip(int8_t{-1});
    EXPECT_EQ(cap.str(), "255\n");
}

TEST(PrintIpTest, Int16)
{
    CoutCapture cap;
    print_ip(int16_t{0});
    EXPECT_EQ(cap.str(), "0.0\n");
}

TEST(PrintIpTest, Int32)
{
    CoutCapture cap;
    print_ip(int32_t{2130706433});
    EXPECT_EQ(cap.str(), "127.0.0.1\n");
}

TEST(PrintIpTest, Int64)
{
    CoutCapture cap;
    print_ip(int64_t{8875824491850138409});
    EXPECT_EQ(cap.str(), "123.45.67.89.101.112.131.41\n");
}

//
// --- STRING ---
//
//------------------------------------------------------------------------------

TEST(PrintIpTest, String)
{
    CoutCapture cap;
    print_ip(std::string{"Hello, World!"});
    EXPECT_EQ(cap.str(), "Hello, World!\n");
}

//
// --- VECTOR ---
//
//------------------------------------------------------------------------------

TEST(PrintIpTest, Vector)
{
    CoutCapture cap;
    print_ip(std::vector<int>{100, 200, 300, 400});
    EXPECT_EQ(cap.str(), "100.200.300.400\n");
}

//
// --- LIST ---
//
//------------------------------------------------------------------------------

TEST(PrintIpTest, List)
{
    CoutCapture cap;
    print_ip(std::list<short>{400, 300, 200, 100});
    EXPECT_EQ(cap.str(), "400.300.200.100\n");
}

//
// --- TUPLE ---
//
//------------------------------------------------------------------------------

TEST(PrintIpTest, Tuple)
{
    CoutCapture cap;
    print_ip(std::make_tuple(123, 456, 789, 0));
    EXPECT_EQ(cap.str(), "123.456.789.0\n");
}

//------------------------------------------------------------------------------