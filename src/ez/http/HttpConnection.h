#pragma once

#include <memory>
#include <string>
#include <vector>

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
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
                   std::vector<HttpWebSocketHandler *> &);

    void start();

    boost::asio::ip::tcp::socket &getSocket();
    void setUserData(void *);
    void *getUserData();

  private:
    // TODO: improve architecture to reduce all the class interconnections
    friend class HttpResponse;
    friend class HttpWebSocketParser;
    friend class HttpWebSocketHandler;
    friend class HttpWebSocketResponse;
    void write(boost::asio::const_buffer buffer);
    void write(std::vector<boost::asio::const_buffer> buffers);

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

    http_parser_settings settings_;
    http_parser parser_;
    boost::array<char, 4096> buf_;
    boost::asio::ip::tcp::socket socket_;
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
