project(ez-http)
cmake_minimum_required(VERSION 2.8.12)

if(NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
endif()

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0600)
endif()

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include_directories(${CMAKE_SOURCE_DIR}/src)

# boost defines
add_definitions(-DBOOST_IOSTREAMS_NO_LIB)

add_library(${PROJECT_NAME}
    src/ez/http/HttpConnection.h
    src/ez/http/HttpConnection.cpp
    src/ez/http/HttpCorsHandler.h
    src/ez/http/HttpCorsHandler.cpp
    src/ez/http/HttpFileHandler.h
    src/ez/http/HttpFileHandler.cpp
    src/ez/http/HttpHeader.h
    src/ez/http/HttpResponse.h
    src/ez/http/HttpResponse.cpp
    src/ez/http/HttpRequest.h
    src/ez/http/HttpRequestHandler.h
    src/ez/http/HttpServer.h
    src/ez/http/HttpServer.cpp
    src/ez/http/HttpStatus.h
    src/ez/http/HttpWebSocketHandler.h
    src/ez/http/HttpWebSocketParser.h
    src/ez/http/HttpWebSocketParser.cpp
    src/ez/http/HttpWebSocketRequest.h
    src/ez/http/HttpWebSocketResponse.h
    src/ez/http/HttpWebSocketResponse.cpp
)

target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})

add_executable(ExamplesHelloWorld examples/HelloWorld.cpp)
target_link_libraries(ExamplesHelloWorld ${CONAN_LIBS} ${PROJECT_NAME})
