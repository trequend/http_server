// Copyright 2024 Balakin Dmitry. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include <winsock2.h>

#include <cstring>
#include <iostream>

#include "scope_guard.h"
#include "simple_http.h"

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
        void* client_native_socket = server->accept(accept_error);
        if (accept_error != simple_http::Server::AcceptError::kOk) {
            std::cout << "Accept error: " << static_cast<int>(accept_error)
                      << std::endl;
            continue;
        }

        std::cout << "Client connected" << std::endl;
        ::SOCKET client_socket =
            reinterpret_cast<::SOCKET>(client_native_socket);
        ScopeGuard socket_guard([&]() {
            ::closesocket(client_socket);
            std::cout << "Client socket was closed" << std::endl;
        });

        unsigned long timeout = 1000;  // 1 sec
        if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout,
                       sizeof(timeout)) == SOCKET_ERROR) {
            std::cout << "Setsockopt error" << std::endl;
            continue;
        }

        char recv_buffer[4096];
        int request_bytes_count =
            ::recv(client_socket, recv_buffer, sizeof(recv_buffer), 0);
        if (request_bytes_count == SOCKET_ERROR) {
            int error = ::WSAGetLastError();
            if (error == WSAETIMEDOUT) {
                std::cerr << "Timeout occurred while receiving data"
                          << std::endl;
            } else {
                std::cerr << "Error receiving data: " << error << std::endl;
            }

            continue;
        }

        printf("Request:\n%.*s\n", request_bytes_count, recv_buffer);

        const char* response =
            "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nContent-Length: "
            "12\r\n\r\nHello world!";
        printf("Response:\n%s\n\n", response);
        ::send(client_socket, response, static_cast<int>(strlen(response)), 0);
    }

    return EXIT_SUCCESS;
}
