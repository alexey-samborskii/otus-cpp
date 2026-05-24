#include "Server.hpp"

#include "Session.hpp"

#include <boost/system/error_code.hpp>

#include <iostream>
#include <memory>
#include <utility>

//------------------------------------------------------------------------------

Server::Server(
    boost::asio::io_context& io_context,
    unsigned short           port,
    std::size_t              bulk_size)
    : acceptor_(
          io_context,
          Tcp::endpoint(Tcp::v4(), port))
    , bulk_size_(bulk_size)
    , static_handle_(async::connect(bulk_size_))
    , active_sessions_(0)
{
    doAccept();
}

//------------------------------------------------------------------------------

Server::~Server()
{
    std::lock_guard<std::mutex> lock(static_handle_mutex_);

    if (static_handle_ != nullptr)
    {
        async::disconnect(static_handle_);
        static_handle_ = nullptr;
    }
}

//------------------------------------------------------------------------------

void Server::deliverStaticLine(const std::string& line)
{
    std::lock_guard<std::mutex> lock(static_handle_mutex_);

    if (static_handle_ == nullptr)
    {
        static_handle_ = async::connect(bulk_size_);
    }

    async::receive(
        static_handle_,
        line.data(),
        line.size());
}

//------------------------------------------------------------------------------

void Server::flushStaticBlock()
{
    std::lock_guard<std::mutex> lock(static_handle_mutex_);

    if (static_handle_ != nullptr)
    {
        async::disconnect(static_handle_);
    }

    static_handle_ = async::connect(bulk_size_);
}

//------------------------------------------------------------------------------

void Server::onSessionClosed()
{
    if (active_sessions_ == 0)
    {
        return;
    }

    --active_sessions_;

    if (active_sessions_ == 0)
    {
        flushStaticBlock();
    }
}

//------------------------------------------------------------------------------

std::size_t Server::bulkSize() const
{
    return bulk_size_;
}

//------------------------------------------------------------------------------

void Server::doAccept()
{
    acceptor_.async_accept(
        [this](const boost::system::error_code& error, Tcp::socket socket) {
            if (!error)
            {
                ++active_sessions_;

                std::make_shared<Session>(
                    std::move(socket),
                    *this)
                    ->start();
            }
            else
            {
                std::cerr << "Accept error: " << error.message() << '\n';
            }

            doAccept();
        });
}