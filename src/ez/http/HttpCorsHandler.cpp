#include "ez/http/HttpCorsHandler.h"

#include "ez/http/HttpRequest.h"
#include "ez/http/HttpResponse.h"

namespace ez {
namespace http {

HttpCorsHandler::HttpCorsHandler(const std::string &origin) : origin_(origin) {}

bool HttpCorsHandler::handleRequest(const HttpRequest &req, HttpResponse &res) {
    if (!origin_.empty()) {
        res.addHeader("Access-Control-Allow-Origin", origin_);
    }
    return false; // false indicates that subsequent Request Handler are allowed
}

} // namespace http
} // namespace ez
