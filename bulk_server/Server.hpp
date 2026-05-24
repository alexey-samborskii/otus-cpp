#pragma once

#include <async.h>

#include <boost/asio.hpp>

#include <cstddef>
#include <mutex>
#include <string>

//------------------------------------------------------------------------------

class Server
{
public:
    Server(
        boost::asio::io_context& io_context,
        unsigned short           port,
        std::size_t              bulk_size);

    ~Server();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    void deliverStaticLine(const std::string& line);
    void flushStaticBlock();
    void onSessionClosed();

    std::size_t bulkSize() const;

private:
    using Tcp = boost::asio::ip::tcp;

    void doAccept();

    Tcp::acceptor acceptor_;
    std::size_t   bulk_size_;

    std::mutex      static_handle_mutex_;
    async::handle_t static_handle_;

    std::size_t active_sessions_;
};