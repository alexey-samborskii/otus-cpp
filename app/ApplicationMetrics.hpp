#pragma once

#include "tasks/TaskScheduler.hpp"

#include <atomic>
#include <cstdint>
#include <string>

namespace application
{

class ApplicationMetrics
{
public:
    void recordRequest();
    void recordResponse(unsigned int status);
    void recordException();

    std::string toPrometheusText(
        const tasks::TaskSchedulerMetrics &scheduler_metrics) const;

private:
    std::atomic_uint64_t http_requests_{0};
    std::atomic_uint64_t http_responses_2xx_{0};
    std::atomic_uint64_t http_responses_3xx_{0};
    std::atomic_uint64_t http_responses_4xx_{0};
    std::atomic_uint64_t http_responses_5xx_{0};
    std::atomic_uint64_t http_exceptions_{0};
};

} // namespace application
