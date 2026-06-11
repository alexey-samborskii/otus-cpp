#include "server/SslContext.hpp"

namespace ssl = boost::asio::ssl;

namespace server
{

void configureServerSslContext(
    ssl::context                 &ssl_context,
    const std::filesystem::path  &cert_file,
    const std::filesystem::path  &key_file)
{
    ssl_context.set_options(
        ssl::context::default_workarounds |
        ssl::context::no_sslv2 |
        ssl::context::no_sslv3 |
        ssl::context::single_dh_use);

    ssl_context.use_certificate_chain_file(
        cert_file.string());

    ssl_context.use_private_key_file(
        key_file.string(),
        ssl::context::pem);
}

} // namespace server