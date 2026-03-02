#include <gtest/gtest.h>
#include "ip_filter.hpp"
#include <fstream>

//------------------------------------------------------------------------------

TEST(IpAddressTest, ConstructFromOctets)
{
    IpAddress ip(192, 168, 1, 10);
    auto      oct = ip.ip();

    EXPECT_EQ(oct[0], 192);
    EXPECT_EQ(oct[1], 168);
    EXPECT_EQ(oct[2], 1);
    EXPECT_EQ(oct[3], 10);
}

//------------------------------------------------------------------------------

TEST(IpAddressTest, ConstructFromString)
{
    IpAddress ip("127.0.0.1");
    auto      oct = ip.ip();

    EXPECT_EQ(oct[0], 127);
    EXPECT_EQ(oct[1], 0);
    EXPECT_EQ(oct[2], 0);
    EXPECT_EQ(oct[3], 1);
}

//------------------------------------------------------------------------------

inline IpStorage loadIpStorage()
{
    const std::string file_name = "ip_filter.tsv";
    IpStorage         storage;
    std::ifstream     ifs(file_name);
    for (std::string line; std::getline(ifs, line);)
    {
        auto pos    = line.find('\t');
        auto ip_str = (pos == std::string::npos) ?
                          std::string_view(line) :
                          std::string_view(line).substr(0, pos);
        storage.emplace(ip_str);
    }
    return storage;
}

//------------------------------------------------------------------------------

TEST(IpAddressTest, FilterByFirstByte)
{
    IpStorage storage     = loadIpStorage();
    auto      ip          = storage.filter_ip(1);
    auto      ip_expected = std::list<IpAddress>{IpAddress("1.231.69.33"),
                                                 IpAddress("1.87.203.225"),
                                                 IpAddress("1.70.44.170"),
                                                 IpAddress("1.29.168.152"),
                                                 IpAddress("1.1.234.8")};

    EXPECT_EQ(ip, ip_expected);
}

//------------------------------------------------------------------------------

TEST(IpAddressTest, FilterByFirstAndSecondBytes)
{
    IpStorage storage     = loadIpStorage();
    auto      ip          = storage.filter_ip(46, 70);
    auto      ip_expected = std::list<IpAddress>{IpAddress("46.70.225.39"),
                                                 IpAddress("46.70.147.26"),
                                                 IpAddress("46.70.113.73"),
                                                 IpAddress("46.70.29.76")};

    EXPECT_EQ(ip, ip_expected);
}

//------------------------------------------------------------------------------

TEST(IpAddressTest, FilterByAnyByte)
{
    IpStorage storage = loadIpStorage();

    auto ip          = storage.filter_any(46);
    auto ip_expected = std::list<IpAddress>{IpAddress("186.204.34.46"),
                                            IpAddress("186.46.222.194"),
                                            IpAddress("185.46.87.231"),
                                            IpAddress("185.46.86.132"),
                                            IpAddress("185.46.86.131"),
                                            IpAddress("185.46.86.131"),
                                            IpAddress("185.46.86.22"),
                                            IpAddress("185.46.85.204"),
                                            IpAddress("185.46.85.78"),
                                            IpAddress("68.46.218.208"),
                                            IpAddress("46.251.197.23"),
                                            IpAddress("46.223.254.56"),
                                            IpAddress("46.223.254.56"),
                                            IpAddress("46.182.19.219"),
                                            IpAddress("46.161.63.66"),
                                            IpAddress("46.161.61.51"),
                                            IpAddress("46.161.60.92"),
                                            IpAddress("46.161.60.35"),
                                            IpAddress("46.161.58.202"),
                                            IpAddress("46.161.56.241"),
                                            IpAddress("46.161.56.203"),
                                            IpAddress("46.161.56.174"),
                                            IpAddress("46.161.56.106"),
                                            IpAddress("46.161.56.106"),
                                            IpAddress("46.101.163.119"),
                                            IpAddress("46.101.127.145"),
                                            IpAddress("46.70.225.39"),
                                            IpAddress("46.70.147.26"),
                                            IpAddress("46.70.113.73"),
                                            IpAddress("46.70.29.76"),
                                            IpAddress("46.55.46.98"),
                                            IpAddress("46.49.43.85"),
                                            IpAddress("39.46.86.85"),
                                            IpAddress("5.189.203.46")};

    EXPECT_EQ(ip, ip_expected);
}

//------------------------------------------------------------------------------