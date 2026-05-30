#pragma once

#include "CommandProcessor.hpp"
#include "Database.hpp"

#include <boost/asio.hpp>

#include <memory>
#include <string>

//------------------------------------------------------------------------------

class Session : public std::enable_shared_from_this<Session>
{
public:
    using Tcp = boost::asio::ip::tcp;

    Session(Tcp::socket socket, std::shared_ptr<Database> database);

    void start();

private:
    void doRead();
    void doWrite(std::string response);
    void stop();

    static void removeTrailingCarriageReturn(std::string& line);

private:
    Tcp::socket            socket_;
    boost::asio::streambuf read_buffer_;
    CommandProcessor       command_processor_;
    bool                   stopped_;
};