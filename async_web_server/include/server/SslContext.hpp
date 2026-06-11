#pragma once

#include <boost/asio/ssl.hpp>

#include <filesystem>

namespace server
{

void configureServerSslContext(
    boost::asio::ssl::context       &ssl_context,
    const std::filesystem::path     &cert_file,
    const std::filesystem::path     &key_file);

} // namespace server