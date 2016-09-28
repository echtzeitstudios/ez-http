#pragma once

#include <string>

#include "ez/http/HttpFileHandler.h"

namespace ez {
namespace http {

class HttpSingleFileHandler : public HttpFileHandler {
  public:
    explicit HttpSingleFileHandler(const std::string &file);

  protected:
    virtual boost::optional<std::string>
    resolve_path(const std::string &) override;

    std::string file_;
};

} // namespace http
} // namespace ez
