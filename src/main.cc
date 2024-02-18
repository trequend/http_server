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

    simple_http::CreateSocketError create_socket_error;
    std::unique_ptr<simple_http::Socket> socket =
        simple_http::Socket::createSocket(create_socket_error);
    if (create_socket_error != simple_http::CreateSocketError::kOk) {
        std::cout << "Create socket error: "
                  << static_cast<int>(create_socket_error) << std::endl;
        return EXIT_FAILURE;
    }

    simple_http::BindSocketOptions bind_options;
    bind_options.address = "127.0.0.1";  // localhost
    bind_options.port = 3000;
    simple_http::BindSocketError bind_socket_error = socket->bind(bind_options);
    if (bind_socket_error != simple_http::BindSocketError::kOk) {
        std::cout << "Bind socket error: "
                  << static_cast<int>(bind_socket_error) << std::endl;
        return EXIT_FAILURE;
    }

    simple_http::ListenSocketOptions listen_options;
    listen_options.max_connections = 100;
    simple_http::ListenSocketError listen_socket_error =
        socket->listen(listen_options);
    if (listen_socket_error != simple_http::ListenSocketError::kOk) {
        std::cout << "Listen socket error: "
                  << static_cast<int>(listen_socket_error) << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Listening port 3000..." << std::endl;

    while (true) {
        simple_http::AcceptSocketError accept_socket_error;
        void* client_native_socket = socket->accept(accept_socket_error);
        if (accept_socket_error != simple_http::AcceptSocketError::kOk) {
            std::cout << "Accept socket error: "
                      << static_cast<int>(accept_socket_error) << std::endl;
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