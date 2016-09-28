#include "ez/http/HttpFileHandler.h"

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

#include <boost/log/trivial.hpp>

#include "ez/http/HttpRequest.h"
#include "ez/http/HttpResponse.h"

namespace ez {
namespace http {

HttpFileHandler::HttpFileHandler(const std::string &root) : root_(root) {}

// TODO: use boost::asio overlap feature to stream response directly from disk
bool HttpFileHandler::handleRequest(const HttpRequest &req, HttpResponse &res) {
    if (req.method != HTTP_GET) {
        return false;
    }

    // TODO: extract path from url.
    auto request_path = req.uri;
    if (request_path.empty() || request_path[0] != '/' ||
        request_path.find("..") != std::string::npos) {
        res.send(HttpStatus::bad_request);
        return true;
    }

    auto resolved_path_opt = resolve_path(std::move(request_path));
    if (!resolved_path_opt) {
        return false;
    }

    auto &resolved_path = resolved_path_opt.get();
    std::ifstream is(resolved_path.c_str(), std::ios::in | std::ios::binary);
    if (!is) {
        return false;
    }

    // Determine the file extension.
    auto last_slash_pos = resolved_path.find_last_of("/");
    auto last_dot_pos = resolved_path.find_last_of(".");
    auto extension = std::string();
    if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos) {
        extension = resolved_path.substr(last_dot_pos + 1);
    }

    std::string body;
    char buf[2048];
    while (is.read(buf, sizeof(buf)).gcount() > 0) {
        body.append(buf, is.gcount());
    }

    res.addHeader("Content-Type", determine_mime_type(extension));
    res.send(HttpStatus::ok, body);

    return true;
}

boost::optional<std::string>
HttpFileHandler::resolve_path(const std::string &request_path) {
    auto resolved_path = root_ + request_path;
    // If path ends in slash (i.e. is a directory) then add "index.html".
    if (resolved_path[resolved_path.size() - 1] == '/') {
        resolved_path += "index.html";
    }
    return resolved_path;
}

std::string HttpFileHandler::determine_mime_type(const std::string &extension) {
    // clang-format off
    static std::unordered_map<std::string, std::string> MIME_TYPES = {
        {"css", "text/css"},
        {"gif", "image/gif"},
        {"htm", "text/html"},
        {"html", "text/html"},
        {"js", "application/javascript"},
        {"jpg", "image/jpeg"},
        {"png", "image/png"},
        {"proto", "application/protobuf"},
    };
    // clang-format on

    const auto it = MIME_TYPES.find(extension);
    return it != MIME_TYPES.end() ? it->second : "text/plain";
}

} // namespace http
} // namespace ez
