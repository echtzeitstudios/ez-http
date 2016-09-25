#include "ez/http/HttpConnection.h"

#include <boost/bind.hpp>
#include <boost/log/trivial.hpp>

#include <http_parser.h>

#include "ez/http/HttpRequest.h"
#include "ez/http/HttpRequestHandler.h"
#include "ez/http/HttpResponse.h"
#include "ez/http/HttpWebSocketHandler.h"

namespace ez {
namespace http {

HttpConnection::HttpConnection(boost::asio::io_service &io_service,
                               std::vector<HttpRequestHandler *> &req_handlers,
                               std::vector<HttpWebSocketHandler *> &ws_handlers)
    : socket_(io_service), req_handlers_(req_handlers),
      ws_handlers_(ws_handlers), user_data_(nullptr) {
    http_parser_settings_init(&settings_);
    settings_.on_message_begin = HttpConnection::on_message_begin_cb;
    settings_.on_message_complete = &HttpConnection::on_message_complete_cb;
    settings_.on_url = HttpConnection::on_url_cb;
    settings_.on_header_field = HttpConnection::on_header_field_cb;
    settings_.on_header_value = HttpConnection::on_header_value_cb;
    settings_.on_body = HttpConnection::on_body_cb;
    http_parser_init(&parser_, HTTP_REQUEST);
    parser_.data = this;
}

void HttpConnection::start() {
    socket_.async_read_some(
        boost::asio::buffer(buf_),
        boost::bind(&HttpConnection::handle_read, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

boost::asio::ip::tcp::socket &HttpConnection::getSocket() { return socket_; }

void HttpConnection::setUserData(void *user_data) { user_data_ = user_data; }
void *HttpConnection::getUserData() { return user_data_; }

void HttpConnection::write(boost::asio::const_buffer buf) {
    boost::asio::async_write(
        socket_, boost::asio::buffer(buf),
        boost::bind(&HttpConnection::handle_write, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

void HttpConnection::write(std::vector<boost::asio::const_buffer> buffers) {
    boost::asio::async_write(
        socket_, buffers,
        boost::bind(&HttpConnection::handle_write, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

void HttpConnection::handle_write(const boost::system::error_code &ec,
                                  size_t bytes_transferred) {
    if (ec) {
        BOOST_LOG_TRIVIAL(error) << "Write error occurred: " << ec.message();
        return;
    }
}

void HttpConnection::handle_read(const boost::system::error_code &ec,
                                 std::size_t bytes_transferred) {
    if (ec == boost::asio::error::eof) {
        if (parser_.upgrade && ws_handler_) {
            ws_handler_->handleClose(*this);
        }
        return;
    }

    if (ec) {
        BOOST_LOG_TRIVIAL(error) << "Read error occurred: " << ec.message();
        getSocket().close();
        return;
    }

    // we upgraded already, which means this is raw WebSocket data
    if (upgraded_) {
        if (!handle_web_socket_read(bytes_transferred)) {
            return;
        }
    } else {
        if (!handle_http_read(bytes_transferred)) {
            return;
        }
    }

    socket_.async_read_some(
        boost::asio::buffer(buf_),
        boost::bind(&HttpConnection::handle_read, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

bool HttpConnection::handle_http_read(std::size_t bytes_transferred) {
    size_t nparsed = http_parser_execute(&parser_, &settings_, buf_.data(),
                                         bytes_transferred);
    if (nparsed != bytes_transferred) {
        BOOST_LOG_TRIVIAL(error) << "Error while parsing incoming http request!";
        getSocket().close();
        return false;
    }
    return true;
}

bool HttpConnection::handle_web_socket_read(std::size_t bytes_transferred) {
    if (!ws_handler_) {
        BOOST_LOG_TRIVIAL(error) << "Received WebSocket message without having a handler!";
        getSocket().close();
        return false;
    }

    if (!ws_parser_.handleMessage(*this, buf_.data(), bytes_transferred,
                                  *ws_handler_)) {
        getSocket().close();
        return false;
    }

    return true;
}

HttpWebSocketHandler *
HttpConnection::determine_web_socket_handler(const HttpRequest &req) {
    for (auto handler : ws_handlers_) {
        if (handler->handleConnection(*this, req)) {
            return handler;
        }
    }
    return nullptr;
}

int HttpConnection::on_message_begin_cb(http_parser *parser) {
    return ((HttpConnection *)parser->data)->on_message_begin(parser);
}

int HttpConnection::on_message_begin(http_parser *parser) {
    request_ = std::make_unique<HttpRequest>();
    return 0;
}

int HttpConnection::on_message_complete_cb(http_parser *parser) {
    return ((HttpConnection *)parser->data)->on_message_complete(parser);
}

int HttpConnection::on_message_complete(http_parser *parser) {
    request_->method = static_cast<http_method>(parser_.method);
    request_->http_version_major = parser_.http_major;
    request_->http_version_minor = parser_.http_minor;

    response_ = std::make_shared<HttpResponse>(*this);

    // we just received the WebSocket handshake request
    if (parser_.upgrade) {
        ws_handler_ = determine_web_socket_handler(*request_);
        if (!ws_handler_) {
            BOOST_LOG_TRIVIAL(error) << "No appropriate WebSocket handler available!";

            response_->send(HttpStatus::not_found);
            getSocket().close();
            return 0;
        }

        if (!ws_parser_.handleHandshake(*request_, *response_)) {
            BOOST_LOG_TRIVIAL(error) << "Unable to handle WebSocket handshake!";
            response_->send(HttpStatus::bad_request);
            getSocket().close();
            return 0;
        }

        // TODO: is this already the ready state?
        ws_handler_->handleReadyState(*this);
        upgraded_ = true;
        return 0;
    }

    if (http_should_keep_alive(parser) == 0) {
        response_->addHeader("Connection", "close");
    }

    auto handled = false;
    for (auto handler : req_handlers_) {
        if (handler->handleRequest(*request_, *response_)) {
            handled = true;
            // TODO: check whether response was sent?
            break;
        }
    }

    if (!handled) {
        response_->send(HttpStatus::not_found);
    }

    // TODO: correct continue http status 100 handling
    // TODO: correct keep-alive handling!

    return 0;
}

int HttpConnection::on_url_cb(http_parser *parser, const char *at,
                              size_t length) {
    return ((HttpConnection *)parser->data)->on_url(parser, at, length);
}

int HttpConnection::on_url(http_parser *parser, const char *at, size_t length) {
    request_->uri.append(at, length);
    return 0;
}

int HttpConnection::on_header_field_cb(http_parser *parser, const char *at,
                                       size_t length) {
    return ((HttpConnection *)parser->data)
        ->on_header_field(parser, at, length);
}

int HttpConnection::on_header_field(http_parser *parser, const char *at,
                                    size_t length) {
    request_->headers.emplace_back(HttpHeader{std::string(at, length), ""});
    return 0;
}

int HttpConnection::on_header_value_cb(http_parser *parser, const char *at,
                                       size_t length) {
    return ((HttpConnection *)parser->data)
        ->on_header_value(parser, at, length);
}

int HttpConnection::on_header_value(http_parser *parser, const char *at,
                                    size_t length) {
    request_->headers.back().value = std::string(at, length);
    return 0;
}

int HttpConnection::on_body_cb(http_parser *parser, const char *at,
                               size_t length) {
    return ((HttpConnection *)parser->data)->on_body(parser, at, length);
}

int HttpConnection::on_body(http_parser *parser, const char *at,
                            size_t length) {
    request_->body.append(at, length);
    return 0;
}

} // namespace http
} // namespace ez
