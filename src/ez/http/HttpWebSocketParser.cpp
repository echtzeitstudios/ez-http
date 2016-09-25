#include "ez/http/HttpWebSocketParser.h"

#include <string>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/log/trivial.hpp>

#include <openssl/sha.h>

#include "ez/http/HttpConnection.h"
#include "ez/http/HttpRequest.h"
#include "ez/http/HttpResponse.h"
#include "ez/http/HttpWebSocketHandler.h"
#include "ez/http/HttpWebSocketRequest.h"
#include "ez/http/HttpWebSocketResponse.h"

namespace ez {
namespace http {

static std::string sha1(const std::string &data) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char *>(data.data()), data.size(),
         hash);
    return std::string(reinterpret_cast<char *>(&hash[0]), 20);
}

static std::string encode64(const std::string &str) {
    static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                            "abcdefghijklmnopqrstuvwxyz"
                                            "0123456789+/";

    unsigned char const *bytes_to_encode =
        reinterpret_cast<unsigned char const *>(str.data());
    size_t in_len = str.size();

    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) +
                              ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) +
                              ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] =
            ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] =
            ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while ((i++ < 3))
            ret += '=';
    }

    return ret;
}

bool HttpWebSocketParser::handleHandshake(const HttpRequest &req,
                                          HttpResponse &res) {
    std::string clientKey;
    if (!extractClientKey(clientKey, req)) {
        return false;
    }

    res.addHeader("Upgrade", "websocket");
    res.addHeader("Connection", "Upgrade");
    res.addHeader("Sec-WebSocket-Accept", generateAcceptkey(clientKey));
    res.send(HttpStatus::switching_protocols);

    was_incomplete_ = false;

    return true;
}

bool HttpWebSocketParser::extractClientKey(std::string &clientKey,
                                           const HttpRequest &req) {
    static std::string clientKeyHeader = "Sec-WebSocket-Key";
    for (const auto &header : req.headers) {
        if (boost::iequals(header.name, clientKeyHeader)) {
            clientKey = header.value;
            return true;
        }
    }
    return false;
}

std::string
HttpWebSocketParser::generateAcceptkey(const std::string &clientKey) {
    static const auto MAGIC_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    return encode64(sha1(clientKey + MAGIC_GUID));
}

bool HttpWebSocketParser::handleMessage(HttpConnection &conn, char *data,
                                        size_t len,
                                        HttpWebSocketHandler &handler) {
    switch (parseFrame((unsigned char *)data, len)) {
    case ERROR_FRAME:
        BOOST_LOG_TRIVIAL(error) << "ERROR during websocket!";
        return false;

    case CLOSING_FRAME:
        makeFrame(res_buf_, CLOSING_FRAME, nullptr, 0);
        conn.write(boost::asio::buffer(res_buf_));
        return false; // TODO: is the client responsible for closing the
                      // connection?

    case TEXT_FRAME: {
        HttpWebSocketRequest request{false, req_buf_};
        HttpWebSocketResponse response{conn, *this};
        handler.handleMessage(conn, request, response);
        return true;
    }

    case BINARY_FRAME: {
        HttpWebSocketRequest request{true, req_buf_};
        HttpWebSocketResponse response{conn, *this};
        handler.handleMessage(conn, request, response);
        return true;
    }

    case PING_FRAME:
        makeFrame(res_buf_, PONG_FRAME, nullptr, 0);
        conn.write(boost::asio::buffer(res_buf_));
        return true;

    case INCOMPLETE_FRAME:
        return true;

    default:
        BOOST_LOG_TRIVIAL(error) << "Received unknown WebSocket frame.";
        return false;
    }
}

HttpWebSocketParser::FrameType
HttpWebSocketParser::parseFrame(unsigned char *in_buffer, size_t in_length) {
    if (in_length < 3) {
        return FrameType::INCOMPLETE_FRAME;
    }

    if (!was_incomplete_) {
        req_buf_.clear();
    }

    unsigned char msg_opcode = in_buffer[0] & 0x0F;
    unsigned char msg_fin = (in_buffer[0] >> 7) & 0x01;
    unsigned char msg_masked = (in_buffer[1] >> 7) & 0x01;

    int payload_length = 0;
    int pos = 2;
    int length_field = in_buffer[1] & (~0x80);
    unsigned int mask = 0;

    if (length_field <= 125) {
        payload_length = length_field;
    } else if (length_field == 126) { // msglen is 16bit!
        payload_length = in_buffer[2] + (in_buffer[3] << 8);
        pos += 2;
    } else if (length_field == 127) { // msglen is 64bit!
        payload_length = in_buffer[2] + (in_buffer[3] << 8);
        pos += 8;
    }

    if (msg_masked) {
        mask = *((unsigned int *)(in_buffer + pos));
        pos += 4;

        unsigned char *c = in_buffer + pos;
        for (int i = 0; i < payload_length; i++) {
            c[i] = c[i] ^ ((char *)(&mask))[i % 4];
        }
    }

    auto prev_size = req_buf_.size();
    unsigned char *payload = in_buffer + pos;
    req_buf_.reserve(prev_size + payload_length + 1);
    req_buf_.insert(req_buf_.end(), payload, payload + payload_length);

    if (in_length < payload_length + pos) {
        was_incomplete_ = true;
        return FrameType::INCOMPLETE_FRAME;
    }

    was_incomplete_ = false;

    // TODO: add trailing null bytes for text frames?
    // req_buf_.push_back(0);

    switch (msg_opcode) {
    case 0x0:
        return (msg_fin)
                   ? FrameType::TEXT_FRAME
                   : FrameType::INCOMPLETE_TEXT_FRAME; // continuation frame ?
    case 0x1:
        return (msg_fin) ? FrameType::TEXT_FRAME
                         : FrameType::INCOMPLETE_TEXT_FRAME;
    case 0x2:
        return (msg_fin) ? FrameType::BINARY_FRAME
                         : FrameType::INCOMPLETE_BINARY_FRAME;
    case 0x8:
        return FrameType::CLOSING_FRAME;
    case 0x9:
        return FrameType::PING_FRAME;
    case 0xA:
        return FrameType::PONG_FRAME;
    default:
        BOOST_LOG_TRIVIAL(error) << "Received unknown WebSocket opcode "
                                 << msg_opcode;
        return FrameType::ERROR_FRAME;
    }
}

void HttpWebSocketParser::makeFrame(std::vector<unsigned char> &dst,
                                    FrameType frame_type,
                                    const std::string &msg) {
    makeFrame(dst, frame_type, (const unsigned char *)msg.c_str(), msg.size());
}

void HttpWebSocketParser::makeFrame(std::vector<unsigned char> &buffer,
                                    FrameType frame_type,
                                    const unsigned char *msg, size_t msg_len) {
    buffer.clear();
    buffer.reserve(20 + msg_len);
    buffer.push_back((unsigned char)frame_type); // text frame

    size_t size = msg_len;
    if (size <= 125) {
        buffer.push_back((unsigned char)size);
    } else if (size <= 65535) {
        buffer.push_back(126); // 16 bit length follows

        buffer.push_back((size >> 8) & 0xFF); // leftmost first
        buffer.push_back(size & 0xFF);
    } else {                   // >2^16-1 (65535)
        buffer.push_back(127); // 64 bit length follows

        // write 8 bytes length (significant first)

        // since msg_length is int it can be no longer than 4 bytes = 2^32-1
        // padd zeroes for the first 4 bytes
        for (int i = 3; i >= 0; i--) {
            buffer.push_back(0);
        }
        // write the actual 32bit msg_length in the next 4 bytes
        for (int i = 3; i >= 0; i--) {
            buffer.push_back(((size >> 8 * i) & 0xFF));
        }
    }

    buffer.insert(buffer.end(), msg, msg + msg_len);
}

} // namespace http
} // namespace ez
