#pragma once

#include <memory>
#include <vector>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

namespace ez {
namespace http {

class HttpConnection;
class HttpRequestHandler;
class HttpWebSocketHandler;

class HttpServer {
  public:
    HttpServer(boost::asio::io_service &io_service, int port);

    void addRequestHandler(HttpRequestHandler *handler);
    void addWebSocketHandler(HttpWebSocketHandler *handler);

  private:
    void start_accept();
    void handle_accept(std::shared_ptr<HttpConnection> new_connection,
                       const boost::system::error_code &error);

    boost::asio::io_service &io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;

    std::vector<HttpRequestHandler *> req_handlers_;
    std::vector<HttpWebSocketHandler *> ws_handlers_;
};

} // namespace http
} // namespace ez
