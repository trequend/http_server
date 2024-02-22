// Copyright 2024 Balakin Dmitry. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include <simple_http.h>
#include <winsock2.h>

#include <cstring>
#include <iostream>
#include <memory>

#include "scope_guard.h"

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
    listen_options.backlog_size = 100;
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
        simple_http::SocketReader::ReadError read_error;
        simple_http::SocketReader::ReadResult request = reader.read(read_error);
        if (read_error != simple_http::SocketReader::ReadError::kOk) {
            std::cerr << "Read error: " << static_cast<int>(read_error)
                      << std::endl;
            continue;
        }

        printf("Request:\n%.*s\n", static_cast<int>(request.getLength()),
               request.getBuffer());

        const char* response =
            "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nContent-Length: "
            "12\r\n\r\nHello world!";
        printf("Response:\n%s\n\n", response);
        auto send_error = client_socket->send(response, strlen(response));
        if (send_error != simple_http::Socket::SendError::kOk) {
            std::cout << "Send error: " << static_cast<int>(timeout_error)
                      << std::endl;
        }
    }

    return EXIT_SUCCESS;
}
