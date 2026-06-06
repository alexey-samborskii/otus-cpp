#include "resolve_input_path.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{

constexpr std::size_t kClassesCount  = 10;
constexpr std::size_t kPixelsCount   = 784;
constexpr std::size_t kFeaturesCount = kPixelsCount + 1;

using Model = std::vector<std::vector<double>>;

//------------------------------------------------------------------------------

std::string readFileToString(const std::string &file_name)
{
    std::ifstream file(file_name.c_str(), std::ios::in | std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Cannot open file: " + file_name);
    }

    std::ostringstream ss;
    ss << file.rdbuf();

    return ss.str();
}

//------------------------------------------------------------------------------

void replaceSeparators(std::string &text)
{
    for (char &ch : text)
    {
        switch (ch)
        {
        case ',':
        case ';':
        case '\t':
        case '\r':
        case '\n':
            ch = ' ';
            break;

        default:
            break;
        }
    }
}

//------------------------------------------------------------------------------

Model readLogregModel(const std::string &model_file_name)
{
    std::string model_text = readFileToString(model_file_name);
    replaceSeparators(model_text);

    std::istringstream input(model_text);

    Model model(kClassesCount, std::vector<double>(kFeaturesCount, 0.0));

    for (std::size_t class_id = 0; class_id < kClassesCount; ++class_id)
    {
        for (std::size_t feature_id = 0; feature_id < kFeaturesCount; ++feature_id)
        {
            if (!(input >> model[class_id][feature_id]))
            {
                throw std::runtime_error(
                    "Invalid model file. Expected 10 x 785 coefficients.");
            }
        }
    }

    return model;
}

//------------------------------------------------------------------------------

bool parseTestLine(
    const std::string   &line,
    int                 &expected_class,
    std::vector<double> &pixels)
{
    if (line.empty())
    {
        return false;
    }

    std::istringstream input(line);
    std::string        token;

    if (!std::getline(input, token, ','))
    {
        return false;
    }

    expected_class = std::atoi(token.c_str());

    pixels.clear();
    pixels.reserve(kPixelsCount);

    while (std::getline(input, token, ','))
    {
        pixels.push_back(static_cast<double>(std::atoi(token.c_str())));
    }

    if (pixels.size() != kPixelsCount)
    {
        throw std::runtime_error(
            "Invalid test row. Expected label + 784 pixels.");
    }

    return true;
}

//------------------------------------------------------------------------------

int predictClass(
    const Model               &model,
    const std::vector<double> &pixels)
{
    int    best_class = 0;
    double best_score = -std::numeric_limits<double>::infinity();

    for (std::size_t class_id = 0; class_id < kClassesCount; ++class_id)
    {
        const std::vector<double> &coef = model[class_id];

        double score = coef[0];

        for (std::size_t pixel_id = 0; pixel_id < kPixelsCount; ++pixel_id)
        {
            score += coef[pixel_id + 1] * pixels[pixel_id];
        }

        if (score > best_score)
        {
            best_score = score;
            best_class = static_cast<int>(class_id);
        }
    }

    return best_class;
}

//------------------------------------------------------------------------------

double calculateAccuracy(
    const std::string &test_file_name,
    const Model       &model)
{
    std::ifstream test_file(test_file_name.c_str());
    if (!test_file)
    {
        throw std::runtime_error("Cannot open file: " + test_file_name);
    }

    std::size_t total_count   = 0;
    std::size_t correct_count = 0;

    std::string         line;
    std::vector<double> pixels;

    while (std::getline(test_file, line))
    {
        int expected_class = -1;

        if (!parseTestLine(line, expected_class, pixels))
        {
            continue;
        }

        const int predicted_class = predictClass(model, pixels);

        if (predicted_class == expected_class)
        {
            ++correct_count;
        }

        ++total_count;
    }

    if (total_count == 0)
    {
        throw std::runtime_error("Test file is empty.");
    }

    return static_cast<double>(correct_count) / static_cast<double>(total_count);
}

} // namespace

//------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage:\n"
                      << "  " << argv[0] << " <test.csv> <logreg_coef.txt>\n";

            return 1;
        }

        const std::string test_file_name = resolveInputPath(argv[1]);

        const std::string model_file_name = resolveInputPath(argv[2]);

        const auto model = readLogregModel(model_file_name);

        const auto accuracy = calculateAccuracy(test_file_name, model);

        std::cout << std::fixed << std::setprecision(6) << accuracy << std::endl;

        return 0;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    return 0;
}

//------------------------------------------------------------------------------
