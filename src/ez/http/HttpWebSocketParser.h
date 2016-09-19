#pragma once

#include <string>
#include <vector>

namespace ez {
namespace http {

class HttpConnection;
class HttpRequest;
class HttpResponse;
class HttpWebSocketHandler;

class HttpWebSocketParser {
  public:
    enum FrameType {
        ERROR_FRAME = 0xFF00,
        INCOMPLETE_FRAME = 0xFE00,

        OPENING_FRAME = 0x3300,
        CLOSING_FRAME = 0x3400,

        INCOMPLETE_TEXT_FRAME = 0x01,
        INCOMPLETE_BINARY_FRAME = 0x02,

        TEXT_FRAME = 0x81,
        BINARY_FRAME = 0x82,

        PING_FRAME = 0x19,
        PONG_FRAME = 0x1A
    };

    bool handleHandshake(const HttpRequest &req, HttpResponse &res);
    bool handleMessage(HttpConnection &conn, char *data, size_t len,
                       HttpWebSocketHandler &handler);

  private:
    friend class HttpWebSocketResponse;
    bool extractClientKey(std::string &clientKey, const HttpRequest &req);
    std::string generateAcceptkey(const std::string &client_key);
    FrameType parseFrame(unsigned char *in_buffer, size_t in_length);
    void makeFrame(std::vector<unsigned char> &dst, FrameType frame_type,
                   const std::string &msg);
    void makeFrame(std::vector<unsigned char> &dst, FrameType frame_type,
                   const unsigned char *msg, size_t msg_len);

    bool was_incomplete_{false};
    std::vector<unsigned char> req_buf_;
    std::vector<unsigned char> res_buf_;
};

} // namespace http
} // namespace ez
