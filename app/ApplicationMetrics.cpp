#include "ApplicationMetrics.hpp"

#include <sstream>

namespace application
{

//------------------------------------------------------------------------------

void ApplicationMetrics::recordRequest()
{
    http_requests_.fetch_add(1, std::memory_order_relaxed);
}

//------------------------------------------------------------------------------

void ApplicationMetrics::recordResponse(unsigned int status)
{
    if (status >= 200 && status < 300)
    {
        http_responses_2xx_.fetch_add(1, std::memory_order_relaxed);
        return;
    }

    if (status >= 300 && status < 400)
    {
        http_responses_3xx_.fetch_add(1, std::memory_order_relaxed);
        return;
    }

    if (status >= 400 && status < 500)
    {
        http_responses_4xx_.fetch_add(1, std::memory_order_relaxed);
        return;
    }

    if (status >= 500 && status < 600)
    {
        http_responses_5xx_.fetch_add(1, std::memory_order_relaxed);
    }
}

//------------------------------------------------------------------------------

void ApplicationMetrics::recordException()
{
    http_exceptions_.fetch_add(1, std::memory_order_relaxed);
}

//------------------------------------------------------------------------------

std::string ApplicationMetrics::toPrometheusText(
    const tasks::TaskSchedulerMetrics &scheduler_metrics) const
{
    std::ostringstream output;

    output << "# HELP async_task_http_requests_total Total HTTP requests.\n"
           << "# TYPE async_task_http_requests_total counter\n"
           << "async_task_http_requests_total "
           << http_requests_.load(std::memory_order_relaxed)
           << "\n\n"
           << "# HELP async_task_http_responses_total "
              "Total HTTP responses by status class.\n"
           << "# TYPE async_task_http_responses_total counter\n"
           << "async_task_http_responses_total{code_class=\"2xx\"} "
           << http_responses_2xx_.load(std::memory_order_relaxed)
           << '\n'
           << "async_task_http_responses_total{code_class=\"3xx\"} "
           << http_responses_3xx_.load(std::memory_order_relaxed)
           << '\n'
           << "async_task_http_responses_total{code_class=\"4xx\"} "
           << http_responses_4xx_.load(std::memory_order_relaxed)
           << '\n'
           << "async_task_http_responses_total{code_class=\"5xx\"} "
           << http_responses_5xx_.load(std::memory_order_relaxed)
           << "\n\n"
           << "# HELP async_task_http_exceptions_total "
              "Total unhandled request exceptions.\n"
           << "# TYPE async_task_http_exceptions_total counter\n"
           << "async_task_http_exceptions_total "
           << http_exceptions_.load(std::memory_order_relaxed)
           << "\n\n"
           << "# HELP async_task_scheduler_events_total "
              "Total task scheduler events.\n"
           << "# TYPE async_task_scheduler_events_total counter\n"
           << "async_task_scheduler_events_total{event=\"scheduled\"} "
           << scheduler_metrics.scheduled_tasks
           << '\n'
           << "async_task_scheduler_events_total{event=\"cancelled\"} "
           << scheduler_metrics.cancelled_tasks
           << '\n'
           << "async_task_scheduler_events_total{event=\"completed\"} "
           << scheduler_metrics.completed_tasks
           << '\n'
           << "async_task_scheduler_events_total{event=\"failed\"} "
           << scheduler_metrics.failed_tasks
           << '\n'
           << "async_task_scheduler_events_total{event=\"timer_errors\"} "
           << scheduler_metrics.timer_errors
           << '\n';

    return output.str();
}

//------------------------------------------------------------------------------

} // namespace application
