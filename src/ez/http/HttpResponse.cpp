#include "ez/http/HttpResponse.h"

#include <string>

#include "ez/http/HttpConnection.h"

namespace ez {
namespace http {

namespace status_strings {

const std::string switching_protocols = "HTTP/1.1 101 Switching Protocols\r\n";
const std::string ok = "HTTP/1.1 200 OK\r\n";
const std::string created = "HTTP/1.1 201 Created\r\n";
const std::string accepted = "HTTP/1.1 202 Accepted\r\n";
const std::string no_content = "HTTP/1.1 204 No Content\r\n";
const std::string multiple_choices = "HTTP/1.1 300 Multiple Choices\r\n";
const std::string moved_permanently = "HTTP/1.1 301 Moved Permanently\r\n";
const std::string moved_temporarily = "HTTP/1.1 302 Moved Temporarily\r\n";
const std::string not_modified = "HTTP/1.1 304 Not Modified\r\n";
const std::string bad_request = "HTTP/1.1 400 Bad Request\r\n";
const std::string unauthorized = "HTTP/1.1 401 Unauthorized\r\n";
const std::string forbidden = "HTTP/1.1 403 Forbidden\r\n";
const std::string not_found = "HTTP/1.1 404 Not Found\r\n";
const std::string internal_server_error =
    "HTTP/1.1 500 Internal Server Error\r\n";
const std::string not_implemented = "HTTP/1.1 501 Not Implemented\r\n";
const std::string bad_gateway = "HTTP/1.1 502 Bad Gateway\r\n";
const std::string service_unavailable = "HTTP/1.1 503 Service Unavailable\r\n";

boost::asio::const_buffer to_buffer(HttpStatus status) {
    switch (status) {
    case HttpStatus::switching_protocols:
        return boost::asio::buffer(switching_protocols);
    case HttpStatus::ok:
        return boost::asio::buffer(ok);
    case HttpStatus::created:
        return boost::asio::buffer(created);
    case HttpStatus::accepted:
        return boost::asio::buffer(accepted);
    case HttpStatus::no_content:
        return boost::asio::buffer(no_content);
    case HttpStatus::multiple_choices:
        return boost::asio::buffer(multiple_choices);
    case HttpStatus::moved_permanently:
        return boost::asio::buffer(moved_permanently);
    case HttpStatus::moved_temporarily:
        return boost::asio::buffer(moved_temporarily);
    case HttpStatus::not_modified:
        return boost::asio::buffer(not_modified);
    case HttpStatus::bad_request:
        return boost::asio::buffer(bad_request);
    case HttpStatus::unauthorized:
        return boost::asio::buffer(unauthorized);
    case HttpStatus::forbidden:
        return boost::asio::buffer(forbidden);
    case HttpStatus::not_found:
        return boost::asio::buffer(not_found);
    case HttpStatus::internal_server_error:
        return boost::asio::buffer(internal_server_error);
    case HttpStatus::not_implemented:
        return boost::asio::buffer(not_implemented);
    case HttpStatus::bad_gateway:
        return boost::asio::buffer(bad_gateway);
    case HttpStatus::service_unavailable:
        return boost::asio::buffer(service_unavailable);
    default:
        return boost::asio::buffer(internal_server_error);
    }
}

} // namespace status_strings

namespace misc_strings {

const char name_value_separator[] = {':', ' '};
const char crlf[] = {'\r', '\n'};

} // namespace misc_strings

namespace stock_replies {

const char switching_protocols[] = "";
const char ok[] = "";
const char created[] = "<html>"
                       "<head><title>Created</title></head>"
                       "<body><h1>201 Created</h1></body>"
                       "</html>";
const char accepted[] = "<html>"
                        "<head><title>Accepted</title></head>"
                        "<body><h1>202 Accepted</h1></body>"
                        "</html>";
const char no_content[] = "<html>"
                          "<head><title>No Content</title></head>"
                          "<body><h1>204 Content</h1></body>"
                          "</html>";
const char multiple_choices[] = "<html>"
                                "<head><title>Multiple Choices</title></head>"
                                "<body><h1>300 Multiple Choices</h1></body>"
                                "</html>";
const char moved_permanently[] = "<html>"
                                 "<head><title>Moved Permanently</title></head>"
                                 "<body><h1>301 Moved Permanently</h1></body>"
                                 "</html>";
const char moved_temporarily[] = "<html>"
                                 "<head><title>Moved Temporarily</title></head>"
                                 "<body><h1>302 Moved Temporarily</h1></body>"
                                 "</html>";
const char not_modified[] = "<html>"
                            "<head><title>Not Modified</title></head>"
                            "<body><h1>304 Not Modified</h1></body>"
                            "</html>";
const char bad_request[] = "<html>"
                           "<head><title>Bad Request</title></head>"
                           "<body><h1>400 Bad Request</h1></body>"
                           "</html>";
const char unauthorized[] = "<html>"
                            "<head><title>Unauthorized</title></head>"
                            "<body><h1>401 Unauthorized</h1></body>"
                            "</html>";
const char forbidden[] = "<html>"
                         "<head><title>Forbidden</title></head>"
                         "<body><h1>403 Forbidden</h1></body>"
                         "</html>";
const char not_found[] = "<html>"
                         "<head><title>Not Found</title></head>"
                         "<body><h1>404 Not Found</h1></body>"
                         "</html>";
const char internal_server_error[] =
    "<html>"
    "<head><title>Internal Server Error</title></head>"
    "<body><h1>500 Internal Server Error</h1></body>"
    "</html>";
const char not_implemented[] = "<html>"
                               "<head><title>Not Implemented</title></head>"
                               "<body><h1>501 Not Implemented</h1></body>"
                               "</html>";
const char bad_gateway[] = "<html>"
                           "<head><title>Bad Gateway</title></head>"
                           "<body><h1>502 Bad Gateway</h1></body>"
                           "</html>";
const char service_unavailable[] =
    "<html>"
    "<head><title>Service Unavailable</title></head>"
    "<body><h1>503 Service Unavailable</h1></body>"
    "</html>";

std::string to_string(HttpStatus status) {
    switch (status) {
    case HttpStatus::switching_protocols:
        return switching_protocols;
    case HttpStatus::ok:
        return ok;
    case HttpStatus::created:
        return created;
    case HttpStatus::accepted:
        return accepted;
    case HttpStatus::no_content:
        return no_content;
    case HttpStatus::multiple_choices:
        return multiple_choices;
    case HttpStatus::moved_permanently:
        return moved_permanently;
    case HttpStatus::moved_temporarily:
        return moved_temporarily;
    case HttpStatus::not_modified:
        return not_modified;
    case HttpStatus::bad_request:
        return bad_request;
    case HttpStatus::unauthorized:
        return unauthorized;
    case HttpStatus::forbidden:
        return forbidden;
    case HttpStatus::not_found:
        return not_found;
    case HttpStatus::internal_server_error:
        return internal_server_error;
    case HttpStatus::not_implemented:
        return not_implemented;
    case HttpStatus::bad_gateway:
        return bad_gateway;
    case HttpStatus::service_unavailable:
        return service_unavailable;
    default:
        return internal_server_error;
    }
}

} // namespace stock_replies

HttpResponse::HttpResponse(HttpConnection &connection)
    : connection_(connection) {}

void HttpResponse::addHeader(std::string name, std::string value) {
    headers_.emplace_back(std::move(name), std::move(value));
}

void HttpResponse::send(HttpStatus status, const char *data, size_t len) {
    content_ = std::string{data, len};
    sendImpl(status);
}

void HttpResponse::send(HttpStatus status, const std::string &data) {
    content_ = data;
    sendImpl(status);
}

void HttpResponse::send(HttpStatus status, std::string &&data) {
    content_ = std::move(data);
    sendImpl(status);
}

void HttpResponse::send(HttpStatus status) {
    addHeader("Content-Type", "text/html");
    content_ = stock_replies::to_string(status);
    sendImpl(status);
}

void HttpResponse::sendImpl(HttpStatus status) {
    const auto contentLength = content_.size();
    if (contentLength) {
        headers_.emplace_back("Content-Length", std::to_string(contentLength));
    }

    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(status_strings::to_buffer(status));
    for (const auto &header : headers_) {
        buffers.push_back(boost::asio::buffer(header.name));
        buffers.push_back(
            boost::asio::buffer(misc_strings::name_value_separator));
        buffers.push_back(boost::asio::buffer(header.value));
        buffers.push_back(boost::asio::buffer(misc_strings::crlf));
    }
    buffers.push_back(boost::asio::buffer(misc_strings::crlf));
    buffers.push_back(boost::asio::buffer(content_));

    connection_.write(buffers);
}

} // namespace http
} // namespace ez
