// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include <simple_http.h>

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>

void HandleRequest(simple_http::IncomingMessage& request,
                   simple_http::OutgoingMessage& response);

int main() {
    simple_http::HttpServer::Options options;
    options.threads_count = 1;
    simple_http::HttpServer::CreateError create_error;
    auto server =
        simple_http::HttpServer::create(options, HandleRequest, create_error);
    if (create_error != simple_http::HttpServer::CreateError::kOk) {
        std::cerr << "Create error: " << static_cast<int>(create_error)
                  << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Server is running at http://localhost:3000..." << std::endl;

    simple_http::HttpServer::ListenError listen_error;
    listen_error = server->listen(3000);
    if (listen_error != simple_http::HttpServer::ListenError::kOk) {
        std::cerr << "Listen error: " << static_cast<int>(listen_error)
                  << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void HandleRequest(simple_http::IncomingMessage& request,
                   simple_http::OutgoingMessage& response) {
    response.writeHead("200", "OK");
    response.end();
}
