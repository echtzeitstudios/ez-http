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

void HttpServer::addRequestHandler(HttpRequestHandler *handler) {
    req_handlers_.push_back(handler);
}

void HttpServer::addWebSocketHandler(HttpWebSocketHandler *handler) {
    ws_handlers_.push_back(handler);
}

void HttpServer::start_accept() {
    auto new_connection = std::make_shared<HttpConnection>(
        io_service_, std::ref(req_handlers_), std::ref(ws_handlers_));
    acceptor_.async_accept(new_connection->getSocket(),
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
