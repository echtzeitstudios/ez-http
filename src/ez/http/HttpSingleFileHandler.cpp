#include "ez/http/HttpSingleFileHandler.h"

namespace ez {
namespace http {

HttpSingleFileHandler::HttpSingleFileHandler(const std::string &file)
    : HttpFileHandler("."), file_(file) {}

boost::optional<std::string>
HttpSingleFileHandler::resolve_path(const std::string &) {
    return file_;
}

} // namespace http
} // namespace ez
