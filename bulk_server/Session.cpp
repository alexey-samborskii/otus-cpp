#include "Session.hpp"

#include "Server.hpp"

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include <iostream>
#include <utility>

//------------------------------------------------------------------------------

Session::Session(
    boost::asio::ip::tcp::socket socket,
    Server&                      server)
    : socket_(std::move(socket))
    , server_(server)
    , dynamic_handle_(async::connect(server_.bulkSize()))
    , dynamic_depth_(0)
    , stopped_(false)
{
}

//------------------------------------------------------------------------------

Session::~Session()
{
    stop();
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

    socket_.async_read_some(
        boost::asio::buffer(buffer_),
        [this, self](const boost::system::error_code& error,
                     std::size_t                      length) {
            if (!error)
            {
                processData(length);
                doRead();
                return;
            }

            if (error == boost::asio::error::eof)
            {
                if (!pending_line_.empty())
                {
                    processLine(pending_line_);
                    pending_line_.clear();
                }
            }
            else
            {
                std::cerr << "Read error: " << error.message() << '\n';
            }

            stop();
        });
}

//------------------------------------------------------------------------------

void Session::processData(std::size_t length)
{
    for (std::size_t i = 0; i < length; ++i)
    {
        const char ch = buffer_[i];

        if (ch == '\n')
        {
            processLine(pending_line_);
            pending_line_.clear();
        }
        else if (ch != '\r')
        {
            pending_line_ += ch;
        }
    }
}

//------------------------------------------------------------------------------

void Session::processLine(std::string line)
{
    line += '\n';

    if (line == "{\n")
    {
        if (dynamic_depth_ == 0)
        {
            server_.flushStaticBlock();
        }

        ++dynamic_depth_;
        deliverDynamicLine(line);
        return;
    }

    if (line == "}\n")
    {
        if (dynamic_depth_ == 0)
        {
            return;
        }

        deliverDynamicLine(line);
        --dynamic_depth_;
        return;
    }

    if (dynamic_depth_ > 0)
    {
        deliverDynamicLine(line);
        return;
    }

    server_.deliverStaticLine(line);
}

//------------------------------------------------------------------------------

void Session::deliverDynamicLine(const std::string& line)
{
    if (dynamic_handle_ == nullptr)
    {
        dynamic_handle_ = async::connect(server_.bulkSize());
    }

    async::receive(
        dynamic_handle_,
        line.data(),
        line.size());
}

//------------------------------------------------------------------------------

void Session::stop()
{
    if (stopped_)
    {
        return;
    }

    stopped_ = true;

    if (dynamic_handle_ != nullptr)
    {
        async::disconnect(dynamic_handle_);
        dynamic_handle_ = nullptr;
    }

    boost::system::error_code ignored_error;
    socket_.shutdown(
        boost::asio::ip::tcp::socket::shutdown_both,
        ignored_error);
    socket_.close(ignored_error);

    server_.onSessionClosed();
}