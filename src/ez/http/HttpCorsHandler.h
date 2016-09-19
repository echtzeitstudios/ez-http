#pragma once

#include <string>

#include "ez/http/HttpRequestHandler.h"

namespace ez {
namespace http {

class HttpCorsHandler : public HttpRequestHandler {
  public:
    explicit HttpCorsHandler(const std::string &origin);

    virtual bool handleRequest(const HttpRequest &req,
                               HttpResponse &res) override;

  private:
    std::string origin_;
};

} // namespace http
} // namespace ez
