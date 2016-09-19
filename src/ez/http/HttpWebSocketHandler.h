#pragma once

#include <string>

namespace ez {
namespace http {

class HttpConnection;
class HttpRequest;
class HttpWebSocketRequest;
class HttpWebSocketResponse;

class HttpWebSocketHandler {
  public:
    virtual bool handleConnection(const HttpConnection &conn,
                                  const HttpRequest &req) = 0;
    virtual void handleReadyState(HttpConnection &conn) = 0;
    virtual bool handleMessage(http::HttpConnection &conn,
                               const HttpWebSocketRequest &req,
                               HttpWebSocketResponse &res) = 0;
    virtual void handleClose(const HttpConnection &conn) = 0;
};

} // namespace http
} // namespace ez
