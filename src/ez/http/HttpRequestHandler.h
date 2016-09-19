#pragma once

#include <string>

namespace ez {
namespace http {

class HttpRequest;
class HttpResponse;

class HttpRequestHandler {
  public:
    virtual bool handleRequest(const HttpRequest &req, HttpResponse &res) = 0;
};

} // namespace http
} // namespace ez
