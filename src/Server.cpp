#include "Server.hpp"

#include "Session.hpp"

#include <boost/system/error_code.hpp>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

//------------------------------------------------------------------------------

Server::Server(boost::asio::io_context&  io_context,
               unsigned short            port,
               std::shared_ptr<Database> database)
    : acceptor_(io_context, Tcp::endpoint(Tcp::v4(), port))
    , database_(std::move(database))
{
    if (!database_)
    {
        throw std::invalid_argument("Database is null");
    }

    doAccept();
}

//------------------------------------------------------------------------------

void Server::doAccept()
{
    acceptor_.async_accept(
        [this](const boost::system::error_code& error, Tcp::socket socket) {
            if (!error)
            {
                std::make_shared<Session>(std::move(socket), database_)->start();
            }
            else
            {
                std::cerr << "Accept error: " << error.message() << '\n';
            }
            doAccept();
        });
}

//------------------------------------------------------------------------------