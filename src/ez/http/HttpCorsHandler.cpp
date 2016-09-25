#include "ez/http/HttpCorsHandler.h"

#include "ez/http/HttpRequest.h"
#include "ez/http/HttpResponse.h"

namespace ez {
namespace http {

void HttpCorsHandler::setOrigin(const std::string &v) { origin_ = v; }

void HttpCorsHandler::setMethods(const std::string &v) { methods_ = v; }

void HttpCorsHandler::setHeaders(const std::string &v) { headers_ = v; }

bool HttpCorsHandler::handleRequest(const HttpRequest &req, HttpResponse &res) {
    if (!origin_.empty()) {
        res.addHeader("Access-Control-Allow-Origin", origin_);
        res.addHeader("Access-Control-Allow-Methods", methods_);
        res.addHeader("Access-Control-Allow-Headers", headers_);
    }

    if (req.method == HTTP_OPTIONS) { // preflight mode, accept all for now
        res.send(ez::http::HttpStatus::accepted);
        return true;
    }

    return false; // false indicates that subsequent Request Handler are allowed
}

} // namespace http
} // namespace ez
