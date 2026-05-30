#include "Session.hpp"

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include <istream>
#include <utility>

//------------------------------------------------------------------------------

Session::Session(Tcp::socket socket, std::shared_ptr<Database> database)
    : socket_(std::move(socket))
    , command_processor_(std::move(database))
    , stopped_(false)
{
}

//------------------------------------------------------------------------------

void Session::start()
{
    doRead();
}

//------------------------------------------------------------------------------

void Session::doRead()
{
    auto self = shared_from_this();

    boost::asio::async_read_until(
        socket_,
        read_buffer_,
        '\n',
        [this, self](const boost::system::error_code& error,
                     std::size_t) {
            if (error)
            {
                stop();
                return;
            }

            std::istream input(&read_buffer_);

            std::string line;
            std::getline(input, line);

            removeTrailingCarriageReturn(line);

            const auto response = command_processor_.process(line);

            doWrite(response);
        });
}

//------------------------------------------------------------------------------

void Session::doWrite(std::string response)
{
    auto self = shared_from_this();

    auto response_buffer = std::make_shared<std::string>(std::move(response));

    boost::asio::async_write(
        socket_,
        boost::asio::buffer(*response_buffer),
        [this, self, response_buffer](const boost::system::error_code& error,
                                      std::size_t) {
            if (error)
            {
                stop();
                return;
            }

            doRead();
        });
}

//------------------------------------------------------------------------------

void Session::stop()
{
    if (stopped_)
    {
        return;
    }

    stopped_ = true;

    boost::system::error_code ignored_error;

    socket_.shutdown(
        boost::asio::ip::tcp::socket::shutdown_both,
        ignored_error);

    socket_.close(ignored_error);
}

//------------------------------------------------------------------------------

void Session::removeTrailingCarriageReturn(std::string& line)
{
    if (!line.empty() && line.back() == '\r')
    {
        line.pop_back();
    }
}