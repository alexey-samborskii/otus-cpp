#include <cerrno>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

//------------------------------------------------------------------------------

std::vector<std::string> splitCsvLine(const std::string& line)
{
    std::vector<std::string> fields;
    std::string              field;
    bool                     in_quotes = false;

    for (std::size_t i = 0; i < line.size(); ++i)
    {
        const char c = line[i];

        if (c == '"')
        {
            if (in_quotes && i + 1 < line.size() && line[i + 1] == '"')
            {
                field += '"';
                ++i;
            }
            else
            {
                in_quotes = !in_quotes;
            }
        }
        else if (c == ',' && !in_quotes)
        {
            fields.push_back(field);
            field.clear();
        }
        else
        {
            field += c;
        }
    }

    fields.push_back(field);
    return fields;
}

//------------------------------------------------------------------------------

bool parsePrice(const std::string& text, long double& price)
{
    if (text.empty() || text == "price")
    {
        return false;
    }

    char* end = NULL;
    errno     = 0;

    const long double value = std::strtold(text.c_str(), &end);
    if (end == text.c_str() || *end != '\0' || errno == ERANGE)
    {
        return false;
    }

    price = value;
    return true;
}

//------------------------------------------------------------------------------

bool extractPrice(const std::string& line, long double& price)
{
    const std::vector<std::string> fields = splitCsvLine(line);

    // Dataset columns:
    // id,name,host_id,host_name,neighbourhood_group,neighbourhood,
    // latitude,longitude,room_type,price,...
    constexpr std::size_t kPriceColumnIndex = 9;

    if (fields.size() <= kPriceColumnIndex)
    {
        return false;
    }

    return parsePrice(fields[kPriceColumnIndex], price);
}

//------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    std::cout << std::setprecision(21);

    std::string line;
    while (std::getline(std::cin, line))
    {
        long double price = 0.0;
        if (!extractPrice(line, price))
        {
            continue;
        }

        std::cout << "price" << '\t' << price << '\t' << 1 << '\n';
    }

    return 0;
}
