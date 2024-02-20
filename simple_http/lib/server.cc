// Copyright 2024 Balakin Dmitry. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include "server.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <string>

#include "init_socket_library.h"
#include "socket.h"

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

simple_http::Server::BindError simple_http::Server::bind(
    const simple_http::Server::BindOptions& options) {
    using namespace simple_http;

    if (is_binded_) {
        return Server::BindError::kAlreadyBinded;
    }

    unsigned short port = static_cast<unsigned short>(options.port);
    if (options.port < 0 || options.port > 65535) {
        return Server::BindError::kWrongPort;
    }

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

simple_http::Server::ListenError simple_http::Server::listen(
    const simple_http::Server::ListenOptions& options) {
    using namespace simple_http;

    if (is_listening_) {
        return Server::ListenError::kAlreadyListening;
    }

    if (!is_binded_) {
        return Server::ListenError::kNotBinded;
    }

    if (options.backlog_size < 0 || options.backlog_size > SOMAXCONN) {
        return Server::ListenError::kWrongBacklogSize;
    }

    long long connection_timeout = options.connection_timeout.count();
    if (connection_timeout < 0 || connection_timeout > MAXDWORD) {
        return Server::ListenError::kWrongConnectionTimeout;
    }

    ::SOCKET native_socket =
        reinterpret_cast<::SOCKET>(socket_descriptor_.load());
    int listen_result = ::listen(native_socket, options.backlog_size);
    if (listen_result == SOCKET_ERROR) {
        return Server::ListenError::kUnknown;
    }

    if (connection_timeout > 0) {
        unsigned long timeout = static_cast<unsigned long>(connection_timeout);
        if (::setsockopt(native_socket, SOL_SOCKET, SO_RCVTIMEO,
                         reinterpret_cast<const char*>(&timeout),
                         sizeof(timeout)) == SOCKET_ERROR ||
            ::setsockopt(native_socket, SOL_SOCKET, SO_SNDTIMEO,
                         reinterpret_cast<const char*>(&timeout),
                         sizeof(timeout)) == SOCKET_ERROR) {
            return Server::ListenError::kUnknown;
        }
    }

    is_listening_ = true;
    return Server::ListenError::kOk;
}

std::unique_ptr<simple_http::Socket> simple_http::Server::accept(
    simple_http::Server::AcceptError& error) {
    using namespace simple_http;
    if (!is_binded_) {
        error = Server::AcceptError::kNotBinded;
        return nullptr;
    }

    if (!is_listening_) {
        error = Server::AcceptError::kNotListining;
        return nullptr;
    }

    ::SOCKET native_socket =
        reinterpret_cast<::SOCKET>(socket_descriptor_.load());
    ::SOCKET client_native_socket = ::accept(native_socket, nullptr, nullptr);
    if (client_native_socket == INVALID_SOCKET) {
        error = Server::AcceptError::kUnknown;
        return nullptr;
    }

    error = Server::AcceptError::kOk;
    void* client_socket_descriptor = reinterpret_cast<void*>(client_native_socket);
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

std::unique_ptr<simple_http::Server> simple_http::Server::createServer(
    simple_http::Server::CreateError& error) {
    using namespace simple_http;

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

simple_http::Server::~Server() { CloseNativeSocket(socket_descriptor_); }
