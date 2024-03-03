// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include <cassert>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "http_connection_handler.h"

namespace simple_http {

class HttpServer {
   public:
    struct Options {
        std::chrono::milliseconds timeout = std::chrono::milliseconds(1000);
        size_t request_buffer_length = 32768;
        size_t response_buffer_length = 32768;
        size_t threads_count =
            static_cast<size_t>(std::thread::hardware_concurrency());
    };

    enum class CreateServerError {
        kOk = 0,
        kLibraryNotInitialzed = 1,
    };

    enum class ListenError {
        kUnknown = -1,
        kOk = 0,
        kWrongAddress = 1,
        kAddressInUse = 2,
        kNoAccess = 3,
        kPoolCreation = 4
    };

    HttpServer() = delete;

    ~HttpServer();

    static std::unique_ptr<HttpServer> createServer(
        HttpConnectionHandler handler, CreateServerError& error);
    static std::unique_ptr<HttpServer> createServer(
        Options options, HttpConnectionHandler handler,
        CreateServerError& error);

    ListenError listen(int port);
    ListenError listen(int port, std::string hostname);
    ListenError listen(int port, std::string hostname, size_t backlog);

   private:
    struct ThreadState {
        std::vector<char> request_buffer;
        std::vector<char> response_buffer;
    };

    HttpServer(Options options, HttpConnectionHandler handler)
        : options_(options), handler_(handler){};

    Options options_;
    HttpConnectionHandler handler_;

    bool should_cleanup_library_ = false;
};

}  // namespace simple_http
