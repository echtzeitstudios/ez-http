#pragma once

#include <memory>
#include <string>
#include <vector>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/system/error_code.hpp>

#include <http_parser.h>

#include "ez/http/HttpRequest.h"
#include "ez/http/HttpWebSocketParser.h"

namespace ez {
namespace http {

class HttpRequest;
class HttpRequestHandler;
class HttpWebSocketHandler;

class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
  public:
    HttpConnection(boost::asio::io_service &io_service,
                   std::vector<HttpRequestHandler *> &,
                   std::vector<HttpWebSocketHandler *> &,
                   boost::asio::ssl::context *ctx = nullptr);

    void start();
    void close();

    void setUserData(void *);
    void *getUserData();

  private:
    // TODO: improve architecture to reduce all the class interconnections
    friend class HttpResponse;
    friend class HttpServer;
    friend class HttpWebSocketParser;
    friend class HttpWebSocketHandler;
    friend class HttpWebSocketResponse;
    void write(boost::asio::const_buffer buffer);
    void write(std::vector<boost::asio::const_buffer> buffers);

    void handle_handshake(const boost::system::error_code &ec);
    void handle_write(const boost::system::error_code &error,
                      size_t bytes_transferred);
    void handle_read(const boost::system::error_code &ec,
                     std::size_t bytes_transferred);
    bool handle_http_read(std::size_t bytes_transferred);
    bool handle_web_socket_read(std::size_t bytes_transferred);

    HttpWebSocketHandler *determine_web_socket_handler(const HttpRequest &req);

    // http_parser hooks
    static int on_message_begin_cb(http_parser *);
    int on_message_begin(http_parser *);
    static int on_message_complete_cb(http_parser *parser);
    int on_message_complete(http_parser *parser);
    static int on_url_cb(http_parser *, const char *at, size_t length);
    int on_url(http_parser *, const char *at, size_t length);
    static int on_header_field_cb(http_parser *, const char *at, size_t length);
    int on_header_field(http_parser *, const char *at, size_t length);
    static int on_header_value_cb(http_parser *, const char *at, size_t length);
    int on_header_value(http_parser *, const char *at, size_t length);
    static int on_body_cb(http_parser *, const char *at, size_t length);
    int on_body(http_parser *, const char *at, size_t length);

    using ssl_socket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket &>;

    template <typename... Args> void async_read_some(Args &&... a) {
        if (ssl_socket_) {
            ssl_socket_->async_read_some(std::forward<Args>(a)...);
        } else {
            socket_.async_read_some(std::forward<Args>(a)...);
        }
    }

    template <typename... Args> void async_read(Args &&... a) {
        if (ssl_socket_) {
            boost::asio::async_read(*ssl_socket_, std::forward<Args>(a)...);
        } else {
            boost::asio::async_read(socket_, std::forward<Args>(a)...);
        }
    }

    template <typename... Args> void async_write(Args &&... a) {
        if (ssl_socket_) {
            boost::asio::async_write(*ssl_socket_, std::forward<Args>(a)...);
        } else {
            boost::asio::async_write(socket_, std::forward<Args>(a)...);
        }
    }

    http_parser_settings settings_;
    http_parser parser_;
    boost::array<char, 4096> buf_;
    boost::asio::ip::tcp::socket socket_;
    std::unique_ptr<ssl_socket> ssl_socket_;
    bool need_handshake_ = true;
    std::unique_ptr<HttpRequest> request_;
    std::shared_ptr<HttpResponse> response_;
    std::vector<HttpRequestHandler *> &req_handlers_;
    std::vector<HttpWebSocketHandler *> &ws_handlers_;
    HttpWebSocketHandler *ws_handler_;
    HttpWebSocketParser ws_parser_;
    bool upgraded_{false};
    void *user_data_;
};

} // namespace http
} // namespace ez
