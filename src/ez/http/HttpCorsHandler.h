#pragma once

#include <string>

#include "ez/http/HttpRequestHandler.h"

namespace ez {
namespace http {

class HttpCorsHandler : public HttpRequestHandler {
  public:
    void setOrigin(const std::string &);
    void setMethods(const std::string &);
    void setHeaders(const std::string &);

    virtual bool handleRequest(const HttpRequest &req,
                               HttpResponse &res) override;

  private:
    std::string origin_{"*"};
    std::string methods_{"PUT,GET,POST,DELETE,OPTIONS"};
    std::string headers_{"Content-Type"};
};

} // namespace http
} // namespace ez
