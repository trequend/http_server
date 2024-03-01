// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include "server.h"

#include <atomic>
#include <cassert>
#include <chrono>
#include <memory>
#include <string>

#include "init_socket_library.h"
#include "socket.h"

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#endif

namespace simple_http {

#ifdef _WIN32

Server::BindError Server::bind(const Server::BindOptions& options) {
    unsigned short port = static_cast<unsigned short>(options.port);
    assert(!is_binded_);
    assert(options.port >= 0 && options.port <= 65535);

    ::sockaddr_in socket_address;
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = htons(port);

    int address_result =
        ::inet_pton(AF_INET, options.address.c_str(), &socket_address.sin_addr);
    if (address_result != 1) {
        return Server::BindError::kWrongAddress;
    }

    ::SOCKET native_socket =
        reinterpret_cast<::SOCKET>(socket_descriptor_.load());
    int bind_result =
        ::bind(native_socket, reinterpret_cast<::sockaddr*>(&socket_address),
               sizeof(socket_address));
    if (bind_result == SOCKET_ERROR) {
        int error_code = ::WSAGetLastError();
        if (error_code == WSAEADDRINUSE) {
            return Server::BindError::kAddressInUse;
        } else if (error_code == WSAEACCES) {
            return Server::BindError::kNoAccess;
        }

        return Server::BindError::kUnknown;
    }

    is_binded_ = true;
    return Server::BindError::kOk;
}

Server::ListenError Server::listen(const Server::ListenOptions& options) {
    assert(options.backlog >= 0 && options.backlog <= SOMAXCONN);
    assert(is_binded_);
    assert(!is_listening_);

    ::SOCKET native_socket =
        reinterpret_cast<::SOCKET>(socket_descriptor_.load());
    int listen_result =
        ::listen(native_socket, static_cast<int>(options.backlog));
    if (listen_result == SOCKET_ERROR) {
        return Server::ListenError::kUnknown;
    }

    is_listening_ = true;
    return Server::ListenError::kOk;
}

std::unique_ptr<Socket> Server::accept(Server::AcceptError& error) {
    assert(is_binded_);
    assert(is_listening_);

    ::SOCKET native_socket =
        reinterpret_cast<::SOCKET>(socket_descriptor_.load());
    ::SOCKET client_native_socket = ::accept(native_socket, nullptr, nullptr);
    if (client_native_socket == INVALID_SOCKET) {
        error = Server::AcceptError::kUnknown;
        return nullptr;
    }

    error = Server::AcceptError::kOk;
    void* client_socket_descriptor =
        reinterpret_cast<void*>(client_native_socket);
    return std::make_unique<Socket>(client_socket_descriptor);
}

static void* CreateNativeSocket() {
    ::SOCKET native_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (native_socket == INVALID_SOCKET) {
        return nullptr;
    }

    return reinterpret_cast<void*>(native_socket);
}

static void CloseNativeSocket(void* native_socket) {
    ::closesocket(reinterpret_cast<::SOCKET>(native_socket));
}

#endif

std::unique_ptr<Server> Server::createServer(Server::CreateError& error) {
    if (!IsSocketLibraryInitialized()) {
        error = Server::CreateError::kLibraryNotInitialized;
        return nullptr;
    }

    void* native_socket = CreateNativeSocket();
    if (native_socket == nullptr) {
        error = Server::CreateError::kUnknown;
        return nullptr;
    }

    error = Server::CreateError::kOk;
    Server* server = new Server(native_socket);
    return std::unique_ptr<Server>(server);
}

Server::~Server() { CloseNativeSocket(socket_descriptor_); }

}  // namespace simple_http
