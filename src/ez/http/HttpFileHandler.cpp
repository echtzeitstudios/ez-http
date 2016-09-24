#include "ez/http/HttpFileHandler.h"

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

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

    // If path ends in slash (i.e. is a directory) then add "index.html".
    if (request_path[request_path.size() - 1] == '/') {
        request_path += "index.html";
    }

    std::string full_path = root_ + request_path;
    std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
    if (!is) {
        if (errorPage_.empty()) {
            res.send(HttpStatus::not_found);
            return true;
        } else {
            printf("Requesting non existing path %s, returning installed error page instead.\n",
                request_path.c_str());
            is = std::ifstream(errorPage_.c_str(), std::ios::in | std::ios::binary);
            request_path = errorPage_;
        }
    }

    // Determine the file extension.
    auto last_slash_pos = request_path.find_last_of("/");
    auto last_dot_pos = request_path.find_last_of(".");
    auto extension = std::string();
    if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos) {
        extension = request_path.substr(last_dot_pos + 1);
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

bool HttpFileHandler::setFileNotFoundPage(const std::string &errorPath) {
    std::string full_path = root_ + errorPath;
    std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
    if (!is) {
        printf("Could not set non existing error file  %s", full_path.c_str());
        return false;
    }

    errorPage_ = full_path;
    return true;
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
