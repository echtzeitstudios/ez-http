#include "ez/http/HttpWebSocketResponse.h"

#include <string>

#include <boost/asio/buffer.hpp>

#include "ez/http/HttpConnection.h"
#include "ez/http/HttpWebSocketParser.h"

namespace ez {
namespace http {

HttpWebSocketResponse::HttpWebSocketResponse(HttpConnection &connection,
                                             HttpWebSocketParser &parser)
    : connection_(connection), parser_(parser) {}

void HttpWebSocketResponse::sendText(const std::string &data) {
    parser_.makeFrame(parser_.res_buf_, HttpWebSocketParser::TEXT_FRAME, data);
    connection_.write(boost::asio::buffer(parser_.res_buf_));
}

void HttpWebSocketResponse::sendBinary(const std::string &data) {
    parser_.makeFrame(parser_.res_buf_, HttpWebSocketParser::BINARY_FRAME,
                      data);
    connection_.write(boost::asio::buffer(parser_.res_buf_));
}

} // namespace http
} // namespace ez
