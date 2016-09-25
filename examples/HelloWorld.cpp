#include <functional>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>

#include <ez/http/HttpRequestHandler.h>
#include <ez/http/HttpResponse.h>
#include <ez/http/HttpServer.h>

using namespace ez::http;

class HelloWorldHandler : public HttpRequestHandler {
  public:
    virtual bool handleRequest(const HttpRequest &req,
                               HttpResponse &res) override {
        res.send(HttpStatus::ok, "Hello World");
        return true;
    }
};

int main() {
    try {
        boost::asio::io_service io_service;

        HelloWorldHandler helloWorldHandler;
        HttpServer server(io_service, 8080);
        server.addRequestHandler(&helloWorldHandler);

        boost::asio::signal_set signals(io_service);
        signals.add(SIGINT);
        signals.add(SIGTERM);
#if defined(SIGQUIT)
        signals.add(SIGQUIT);
#endif // defined(SIGQUIT)
        signals.async_wait(
            std::bind(&boost::asio::io_service::stop, &io_service));

        io_service.run();
    } catch (std::exception &e) {
        BOOST_LOG_TRIVIAL(fatal) << "exception: " << e.what();
    }
}
