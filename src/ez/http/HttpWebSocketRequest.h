#pragma once

#include <string>
#include <vector>

namespace ez {
namespace http {

class HttpWebSocketRequest {
  public:
    bool binary;
    std::vector<unsigned char> &payload;
};

} // namespace http
} // namespace ez
