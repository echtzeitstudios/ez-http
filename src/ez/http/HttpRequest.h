#pragma once

#include <string>
#include <vector>

#include <http_parser.h>

#include "ez/http/HttpHeader.h"

namespace ez {
namespace http {

class HttpRequest {
  public:
    http_method method;
    std::string uri;
    int http_version_major;
    int http_version_minor;
    int upgrade;
    std::vector<HttpHeader> headers;
    std::string body;
};

} // namespace http
} // namespace ez
