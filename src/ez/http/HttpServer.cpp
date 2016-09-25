#include "ez/http/HttpServer.h"

#include <boost/bind.hpp>

#include "ez/http/HttpConnection.h"
#include "ez/http/HttpRequest.h"
#include "ez/http/HttpRequestHandler.h"
#include "ez/http/HttpWebSocketHandler.h"

namespace ez {
namespace http {

HttpServer::HttpServer(boost::asio::io_service &io_service, int port)
    : io_service_(io_service),
      acceptor_(io_service, boost::asio::ip::tcp::endpoint(
                                boost::asio::ip::tcp::v4(), port)) {
    start_accept();
}

HttpServer::HttpServer(boost::asio::io_service &io_service, int port,
                       const std::string &cert_file,
                       const std::string &key_file, const std::string &dh_file)
    : io_service_(io_service),
      acceptor_(io_service, boost::asio::ip::tcp::endpoint(
                                boost::asio::ip::tcp::v4(), port)),
      ssl_ctx(new boost::asio::ssl::context(
          boost::asio::ssl::context::tlsv12_server)) {
    if (cert_file.empty() || key_file.empty()) {
        throw std::invalid_argument("Valid cert and key files must be provided "
                                    "to start the server with ssl");
    }

    ssl_ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::no_tlsv1 |
                         boost::asio::ssl::context::single_dh_use);
    ssl_ctx->use_certificate_file(cert_file, boost::asio::ssl::context::pem);
    ssl_ctx->use_private_key_file(key_file, boost::asio::ssl::context::pem);
    if (!dh_file.empty()) {
        ssl_ctx->use_tmp_dh_file(dh_file);
    }

    start_accept();
}

void HttpServer::addRequestHandler(HttpRequestHandler *handler) {
    req_handlers_.push_back(handler);
}

void HttpServer::addWebSocketHandler(HttpWebSocketHandler *handler) {
    ws_handlers_.push_back(handler);
}

void HttpServer::start_accept() {
    auto new_connection =
        std::make_shared<HttpConnection>(io_service_, std::ref(req_handlers_),
                                         std::ref(ws_handlers_), ssl_ctx.get());
    acceptor_.async_accept(new_connection->socket_,
                           boost::bind(&HttpServer::handle_accept, this,
                                       new_connection,
                                       boost::asio::placeholders::error));
}

void HttpServer::handle_accept(std::shared_ptr<HttpConnection> new_connection,
                               const boost::system::error_code &error) {
    if (!error) {
        new_connection->start();
    }

    start_accept();
}

} // namespace http
} // namespace ez
