#pragma once

#include "Database.hpp"

#include <boost/asio.hpp>

#include <memory>

//------------------------------------------------------------------------------

class Server
{
public:
    using Tcp = boost::asio::ip::tcp;

    Server(boost::asio::io_context&  io_context,
           unsigned short            port,
           std::shared_ptr<Database> database);

private:
    void doAccept();

private:
    Tcp::acceptor             acceptor_;
    std::shared_ptr<Database> database_;
};