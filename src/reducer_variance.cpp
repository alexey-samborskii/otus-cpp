#include <iomanip>
#include <iostream>
#include <string>

//------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    std::string        key;
    long double        price         = 0.0;
    long double        price_squared = 0.0;
    unsigned long long count         = 0;

    long double        sum         = 0.0;
    long double        sum_squared = 0.0;
    unsigned long long total_count = 0;

    while (std::cin >> key >> price >> price_squared >> count)
    {
        sum += price;
        sum_squared += price_squared;
        total_count += count;
    }

    if (total_count == 0)
    {
        std::cerr << "No valid price values" << std::endl;
        return 1;
    }

    const long double mean = sum / static_cast<long double>(total_count);
    const long double variance =
        sum_squared / static_cast<long double>(total_count) - mean * mean;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "variance" << '\t' << variance << std::endl;

    return 0;
}
