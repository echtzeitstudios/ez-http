#pragma once

#include <string>
#include <vector>

#include <boost/asio.hpp>

#include "ez/http/HttpHeader.h"
#include "ez/http/HttpStatus.h"

namespace ez {
namespace http {

class HttpConnection;

class HttpResponse {
  public:
    HttpResponse(HttpConnection &);

    void addHeader(std::string name, std::string value);

    void send(HttpStatus status, const char *data, size_t len);
    void send(HttpStatus status, const std::string &data);
    void send(HttpStatus status, std::string &&data);
    void send(HttpStatus status);

  private:
    void sendImpl(HttpStatus status);

    HttpConnection &connection_;
    std::vector<HttpHeader> headers_;
    std::string content_;
};

} // namespace http
} // namespace ez
