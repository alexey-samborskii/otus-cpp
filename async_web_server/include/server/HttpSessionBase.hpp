#pragma once

#include "server/HttpRequest.hpp"
#include "server/HttpResponse.hpp"
#include "server/common.hpp"

#include "common/Logger.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>
#include <boost/system/system_error.hpp>

#include <chrono>
#include <cstdint>
#include <exception>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace server
{

class HttpSessionBase
{
protected:
    explicit HttpSessionBase(CallbackHandleRequest request_handler_cb);

    ~HttpSessionBase() = default;

    //--------------------------------------------------------------------------

    template <typename Stream, typename TimeoutStream>
    auto processRequests(Stream        &stream,
                         TimeoutStream &timeout_stream,
                         const char    *log_prefix,
                         bool           ignore_ssl_stream_truncated)
        -> boost::asio::awaitable<void>
    {
        namespace http = boost::beast::http;

        try
        {
            for (;;)
            {
                timeout_stream.expires_after(kSessionTimeout);

                http::request_parser<http::string_body> parser;

                parser.body_limit(kMaxRequestBodySizeBytes);

                co_await http::async_read(
                    stream,
                    buffer_,
                    parser,
                    boost::asio::use_awaitable);

                timeout_stream.expires_never();

                auto request = makeHttpRequest(parser.release());

                auto response = co_await request_handler_cb_(std::move(request));

                auto beast_response = makeBeastResponse(response);

                const bool keep_alive = beast_response.keep_alive();

                timeout_stream.expires_after(kSessionTimeout);

                co_await http::async_write(
                    stream,
                    beast_response,
                    boost::asio::use_awaitable);

                timeout_stream.expires_never();

                if (!keep_alive)
                {
                    break;
                }
            }
        }
        catch (const boost::system::system_error &error)
        {
            if (!shouldIgnoreSystemError(
                    error.code(),
                    ignore_ssl_stream_truncated))
            {
                std::ostringstream message;

                message << log_prefix << " error: " << error.what();

                common::logError(message.str());
            }
        }
        catch (const std::exception &error)
        {
            std::ostringstream message;

            message << log_prefix << " exception: " << error.what();

            common::logError(message.str());
        }
        catch (...)
        {
            std::ostringstream message;

            message << log_prefix << " unknown exception";

            common::logError(message.str());
        }

        co_return;
    }
    //--------------------------------------------------------------------------

private:
    using BeastRequest =
        boost::beast::http::request<boost::beast::http::string_body>;

    using BeastResponse =
        boost::beast::http::response<boost::beast::http::string_body>;

    static auto makeHttpRequest(BeastRequest &&request) -> HttpRequest;

    static auto makeBeastResponse(const HttpResponse &response) -> BeastResponse;

    static bool shouldIgnoreSystemError(
        const boost::system::error_code &error,
        bool                             ignore_ssl_stream_truncated);

private:
    static constexpr std::chrono::minutes kSessionTimeout{1};
    static constexpr std::uint64_t        kMaxRequestBodySizeBytes{1024 * 1024};

    boost::beast::flat_buffer buffer_;
    CallbackHandleRequest     request_handler_cb_;
};

} // namespace server
