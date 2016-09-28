#pragma once

#include <string>

#include <boost/optional.hpp>

#include "ez/http/HttpRequestHandler.h"

namespace ez {
namespace http {

class HttpFileHandler : public HttpRequestHandler {
  public:
    explicit HttpFileHandler(const std::string &root);

    virtual bool handleRequest(const HttpRequest &req,
                               HttpResponse &res) override;

  protected:
    virtual boost::optional<std::string> resolve_path(const std::string &);

  private:
    std::string determine_mime_type(const std::string &extension);

    std::string root_;
};

} // namespace http
} // namespace ez
