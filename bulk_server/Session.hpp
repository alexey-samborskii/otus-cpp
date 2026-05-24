#pragma once

#include <async.h>

#include <boost/asio.hpp>

#include <array>
#include <cstddef>
#include <string>

//------------------------------------------------------------------------------

class Server;

//------------------------------------------------------------------------------

class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(
        boost::asio::ip::tcp::socket socket,
        Server&                      server);

    ~Session();

    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;

    void start();

private:
    using Tcp = boost::asio::ip::tcp;

    void doRead();
    void processData(std::size_t length);
    void processLine(std::string line);
    void deliverDynamicLine(const std::string& line);
    void stop();

    Tcp::socket             socket_;
    Server&                 server_;
    std::array<char, 4096>  buffer_;
    std::string             pending_line_;

    async::handle_t dynamic_handle_;

    std::size_t dynamic_depth_;
    bool        stopped_;
};