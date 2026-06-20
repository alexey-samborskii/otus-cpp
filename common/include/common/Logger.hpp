#pragma once

#include <iosfwd>
#include <mutex>
#include <string>
#include <string_view>

namespace common
{

enum class LogLevel
{
    kDebug = 0,
    kInfo,
    kWarning,
    kError
};

//------------------------------------------------------------------------------

class Logger
{
public:
    static Logger &instance();

    void     setLevel(LogLevel level);
    LogLevel level() const;

    void write(LogLevel level, std::string_view message);

private:
    Logger() = default;

private:
    mutable std::mutex mutex_;
    LogLevel           level_ = LogLevel::kInfo;
};

//------------------------------------------------------------------------------

std::string toString(LogLevel level);
LogLevel    logLevelFromString(const std::string &value);

void setLogLevel(LogLevel level);
void logDebug(std::string_view message);
void logInfo(std::string_view message);
void logWarning(std::string_view message);
void logError(std::string_view message);

//------------------------------------------------------------------------------

} // namespace common
