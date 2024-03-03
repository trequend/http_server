// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include <simple_http.h>

#include <filesystem>
#include <iostream>
#include <string>

const std::filesystem::path kStaticDir =
    std::filesystem::weakly_canonical("www");

const std::filesystem::path kNotFoundPage = kStaticDir / "_404.html";

void HandleRequest(simple_http::IncomingMessage& request,
                   simple_http::OutgoingMessage& response);

int main() {
    if (!std::filesystem::is_directory(kStaticDir)) {
        std::cerr << "No static files directory" << std::endl;
        return EXIT_FAILURE;
    }

    if (!std::filesystem::is_regular_file(kNotFoundPage)) {
        std::cerr << "No not found page" << std::endl;
        return EXIT_FAILURE;
    }

    simple_http::HttpServer::CreateError create_error;
    auto server = simple_http::HttpServer::create(HandleRequest, create_error);
    if (create_error != simple_http::HttpServer::CreateError::kOk) {
        std::cerr << "Create error: " << static_cast<int>(create_error)
                  << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Listen port 3000..." << std::endl;

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
    auto file_path =
        simple_http::GetRequestFilePath(request.getPath(), kStaticDir);
    if (!file_path.has_value()) {
        return simple_http::ResponseWithFile(response, "404", "Not Found",
                                             kNotFoundPage);
    }

    simple_http::ResponseWithFile(response, "200", "OK", file_path.value());
}
