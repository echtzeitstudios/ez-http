#pragma once

#include <string>

namespace ez {
namespace http {

class HttpHeader {
  public:
    HttpHeader(std::string name, std::string value)
        : name(std::move(name)), value(std::move(value)) {}
    std::string name;
    std::string value;
};

} // namespace http
} // namespace ez
