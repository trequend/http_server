// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include <simple_http.h>

#include <fstream>
#include <iostream>

#include "utils.h"

void HandleRequest(simple_http::IncomingMessage& request,
                   simple_http::OutgoingMessage& response);

int main() {
    simple_http::HttpServer::CreateServerError create_error;
    auto server =
        simple_http::HttpServer::createServer(HandleRequest, create_error);
    if (create_error != simple_http::HttpServer::CreateServerError::kOk) {
        std::cout << "Create error: " << static_cast<int>(create_error)
                  << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Listen port 3000..." << std::endl;

    simple_http::HttpServer::ListenError listen_error;
    listen_error = server->listen(3000);
    if (listen_error != simple_http::HttpServer::ListenError::kOk) {
        std::cout << "Listen error: " << static_cast<int>(listen_error)
                  << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void HandleRequest(simple_http::IncomingMessage& request,
                   simple_http::OutgoingMessage& response) {
    std::cout << "=== Connection ===" << std::endl;
    std::cout << "Method: \"" << request.getMethodName() << "\"" << std::endl;
    std::cout << "Href: \"" << request.getHref() << "\"" << std::endl;
    std::cout << "Path: \"" << request.getPath() << "\"" << std::endl;
    std::cout << "Query: \"" << request.getQuery() << "\"" << std::endl;
    std::cout << "Content length: " << request.getContentLength() << std::endl;
    std::cout << "Headers:" << std::endl;
    PrintHeaders(request.getHeaders());
    if (request.getContentLength() > 0) {
        std::cout << "Body:" << std::endl;
        char body_buffer[1025];
        simple_http::MessageBody::ReadError read_error;
        size_t remaining_bytes;
        do {
            remaining_bytes = request.readBody(
                body_buffer, sizeof(body_buffer) - 1, read_error);
            if (read_error != simple_http::MessageBody::ReadError::kOk) {
                return;
            }

            if (remaining_bytes != 0) {
                body_buffer[remaining_bytes] = '\0';
                std::cout << body_buffer;
            } else {
                std::cout << std::endl;
            }
        } while (remaining_bytes != 0);
    } else {
        std::cout << "No body" << std::endl;
    }
    std::cout << "==================" << std::endl;

    std::string file_name =
        request.getPath() == "/" ? "www/index.html" : "www/_404.html";
    std::ifstream file(file_name, std::ios::binary);

    if (!file.is_open()) {
        std::cout << "File opening error" << std::endl;
        return;
    }

    file.seekg(0, std::ios::end);
    std::streampos end_postion = file.tellg();
    size_t file_size = static_cast<size_t>(end_postion);
    file.seekg(0, std::ios::beg);

    simple_http::HttpHeaders& headers = response.getHeaders();
    headers.add("Content-Length", std::to_string(file_size));
    headers.add("Content-Type", "text/html; charset=UTF-8");
    headers.add("X-Powered-By", "simple_http");

    if (request.getPath() == "/") {
        response.writeHead("200", "OK");
    } else {
        response.writeHead("404", "Not Found");
    }

    char read_buffer[1024];
    while (file.good()) {
        file.read(read_buffer, sizeof(read_buffer));
        size_t bytes_readed = file.gcount();
        if (bytes_readed > 0) {
            simple_http::OutgoingMessage::WriteError write_error;
            write_error = response.write(read_buffer, bytes_readed);
            if (write_error != simple_http::OutgoingMessage::WriteError::kOk) {
                std::cout << "Send error: " << static_cast<int>(write_error)
                          << std::endl;
                return;
            }
        }
    }

    if (file.bad() || !file.eof()) {
        std::cout << "File reading error" << std::endl;
        return;
    }

    file.close();
    response.end();
}
