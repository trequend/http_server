// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include <simple_http.h>
#include <winsock2.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>

#include "scope_guard.h"
#include "utils.h"

int main() {
    simple_http::InitLibraryError error = simple_http::InitLibrary();
    if (error != simple_http::InitLibraryError::kOk) {
        return EXIT_FAILURE;
    }

    ScopeGuard http_guard([]() { simple_http::CleanupLibrary(); });

    simple_http::Server::CreateError create_error;
    std::unique_ptr<simple_http::Server> server =
        simple_http::Server::createServer(create_error);
    if (create_error != simple_http::Server::CreateError::kOk) {
        std::cout << "Create error: " << static_cast<int>(create_error)
                  << std::endl;
        return EXIT_FAILURE;
    }

    simple_http::Server::BindOptions bind_options;
    bind_options.address = "127.0.0.1";  // localhost
    bind_options.port = 3000;
    simple_http::Server::BindError bind_error = server->bind(bind_options);
    if (bind_error != simple_http::Server::BindError::kOk) {
        std::cout << "Bind error: " << static_cast<int>(bind_error)
                  << std::endl;
        return EXIT_FAILURE;
    }

    simple_http::Server::ListenOptions listen_options;
    listen_options.backlog_size = SOMAXCONN;
    simple_http::Server::ListenError listen_error =
        server->listen(listen_options);
    if (listen_error != simple_http::Server::ListenError::kOk) {
        std::cout << "Listen error: " << static_cast<int>(listen_error)
                  << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Listening port 3000..." << std::endl;

    while (true) {
        simple_http::Server::AcceptError accept_error;
        std::unique_ptr<simple_http::Socket> client_socket =
            server->accept(accept_error);
        if (accept_error != simple_http::Server::AcceptError::kOk) {
            std::cout << "Accept error: " << static_cast<int>(accept_error)
                      << std::endl;
            continue;
        }

        std::cout << "Client connected" << std::endl;
        ScopeGuard socket_guard([&client_socket]() {
            client_socket->close();
            std::cout << "Client socket was closed" << std::endl;
        });

        std::chrono::milliseconds timeout(1000);  // 1 sec
        auto timeout_error = client_socket->setTimeout(timeout);
        if (timeout_error != simple_http::Socket::SetTimeoutError::kOk) {
            std::cout << "Set timeout error" << static_cast<int>(timeout_error)
                      << std::endl;
            continue;
        }

        char recv_buffer[4096];
        simple_http::SocketReader reader(client_socket.get(), recv_buffer,
                                         sizeof(recv_buffer));

        char response_buffer[4096];
        simple_http::SocketWriter writer(client_socket.get(), response_buffer,
                                         sizeof(response_buffer));

        simple_http::HttpConnection connection(client_socket.get(), reader,
                                               writer);
        connection.proccessRequest([&](simple_http::IncomingMessage& request,
                                       simple_http::OutgoingMessage& response) {
            std::cout << "=== Connection ===" << std::endl;
            std::cout << "Method: \"" << request.getMethodName() << "\""
                      << std::endl;
            std::cout << "Href: \"" << request.getHref() << "\"" << std::endl;
            std::cout << "Path: \"" << request.getPath() << "\"" << std::endl;
            std::cout << "Query: \"" << request.getQuery() << "\"" << std::endl;
            std::cout << "Content length: \"" << request.getContentLength()
                      << "\"" << std::endl;
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
                    if (read_error !=
                        simple_http::MessageBody::ReadError::kOk) {
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
                    if (write_error !=
                        simple_http::OutgoingMessage::WriteError::kOk) {
                        std::cout
                            << "Send error: " << static_cast<int>(write_error)
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
        });
    }

    return EXIT_SUCCESS;
}
