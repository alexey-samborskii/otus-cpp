#include "common/Logger.hpp"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace common
{

namespace
{

//------------------------------------------------------------------------------

std::string makeAsciiLowercase(std::string value)
{
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](unsigned char character) {
            if (character >= 'A' && character <= 'Z')
            {
                return static_cast<char>(character - 'A' + 'a');
            }
            return static_cast<char>(character);
        });

    return value;
}

//------------------------------------------------------------------------------

std::string currentTimestamp()
{
    using Clock = std::chrono::system_clock;

    const auto now  = Clock::now();
    const auto time = Clock::to_time_t(now);

    std::tm local_time{};

#if defined(_WIN32)
    localtime_s(&local_time, &time);
#else
    localtime_r(&time, &local_time);
#endif

    std::ostringstream stream;

    stream << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");

    return stream.str();
}

} // namespace

//------------------------------------------------------------------------------

Logger &Logger::instance()
{
    static Logger logger;
    return logger;
}

//------------------------------------------------------------------------------

void Logger::setLevel(LogLevel level)
{
    std::lock_guard<std::mutex> lock(mutex_);
    level_ = level;
}

//------------------------------------------------------------------------------

LogLevel Logger::level() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return level_;
}

//------------------------------------------------------------------------------

void Logger::write(LogLevel level, std::string_view message)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (static_cast<int>(level) < static_cast<int>(level_))
    {
        return;
    }

    std::ostream &output = (level == LogLevel::kError) ?
                               std::cerr :
                               std::cout;

    output << '[' << currentTimestamp() << "] [" << toString(level) << "] "
           << message
           << '\n';
}

//------------------------------------------------------------------------------

std::string toString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::kDebug:
        return "debug";

    case LogLevel::kInfo:
        return "info";

    case LogLevel::kWarning:
        return "warning";

    case LogLevel::kError:
        return "error";
    }

    throw std::invalid_argument("Unknown log level");
}

//------------------------------------------------------------------------------

LogLevel logLevelFromString(const std::string &value)
{
    const std::string normalized_value = makeAsciiLowercase(value);

    if (normalized_value == "debug")
    {
        return LogLevel::kDebug;
    }

    if (normalized_value == "info")
    {
        return LogLevel::kInfo;
    }

    if (normalized_value == "warning" || normalized_value == "warn")
    {
        return LogLevel::kWarning;
    }

    if (normalized_value == "error")
    {
        return LogLevel::kError;
    }

    throw std::invalid_argument("Unknown log level: " + value);
}

//------------------------------------------------------------------------------

void setLogLevel(LogLevel level)
{
    Logger::instance().setLevel(level);
}

//------------------------------------------------------------------------------

void logDebug(std::string_view message)
{
    Logger::instance().write(LogLevel::kDebug, message);
}

//------------------------------------------------------------------------------

void logInfo(std::string_view message)
{
    Logger::instance().write(LogLevel::kInfo, message);
}

//------------------------------------------------------------------------------

void logWarning(std::string_view message)
{
    Logger::instance().write(LogLevel::kWarning, message);
}

//------------------------------------------------------------------------------

void logError(std::string_view message)
{
    Logger::instance().write(LogLevel::kError, message);
}

//------------------------------------------------------------------------------

} // namespace common
