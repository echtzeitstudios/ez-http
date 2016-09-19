#pragma once

#include <vector>

namespace ez {
namespace http {

class HttpConnection;
class HttpWebSocketParser;

class HttpWebSocketResponse {
  public:
    HttpWebSocketResponse(HttpConnection &, HttpWebSocketParser &);

    void sendText(const std::string &data);
    void sendBinary(const std::string &data);

  private:
    HttpConnection &connection_;
    HttpWebSocketParser &parser_;
};

} // namespace http
} // namespace ez
