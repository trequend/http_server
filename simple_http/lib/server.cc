// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include "server.h"

#include <atomic>
#include <cassert>
#include <chrono>
#include <memory>
#include <optional>
#include <string>

#include "init_socket_library.h"
#include "socket.h"
#include "socket_descriptor.h"

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#elif __linux__

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

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

    int bind_result = ::bind(socket_descriptor_,
                             reinterpret_cast<::sockaddr*>(&socket_address),
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

    int listen_result =
        ::listen(socket_descriptor_, static_cast<int>(options.backlog));
    if (listen_result == SOCKET_ERROR) {
        return Server::ListenError::kUnknown;
    }

    is_listening_ = true;
    return Server::ListenError::kOk;
}

std::unique_ptr<Socket> Server::accept(Server::AcceptError& error) {
    assert(is_binded_);
    assert(is_listening_);

    SocketDescriptor client_socket_descriptor =
        ::accept(socket_descriptor_, nullptr, nullptr);
    if (client_socket_descriptor == INVALID_SOCKET) {
        int inner_error = ::WSAGetLastError();
        if (inner_error == WSAEINTR) {
            error = Server::AcceptError::kInterrupt;
        } else {
            error = Server::AcceptError::kUnknown;
        }

        return nullptr;
    }

    error = Server::AcceptError::kOk;
    return std::make_unique<Socket>(client_socket_descriptor);
}

static std::optional<SocketDescriptor> CreateNativeSocket() {
    SocketDescriptor socket_descriptor =
        ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_descriptor == INVALID_SOCKET) {
        return std::nullopt;
    }

    return socket_descriptor;
}

static void CloseNativeSocket(SocketDescriptor socket_descriptor) {
    ::closesocket(socket_descriptor);
}

#elif __linux__

constexpr int kInvalidSocket = -1;

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

    int bind_result = ::bind(socket_descriptor_,
                             reinterpret_cast<::sockaddr*>(&socket_address),
                             sizeof(socket_address));
    if (bind_result == kInvalidSocket) {
        if (errno == EADDRINUSE) {
            return Server::BindError::kAddressInUse;
        } else if (errno == EACCES) {
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

    int listen_result =
        ::listen(socket_descriptor_, static_cast<int>(options.backlog));
    if (listen_result == kInvalidSocket) {
        return Server::ListenError::kUnknown;
    }

    is_listening_ = true;
    return Server::ListenError::kOk;
}

std::unique_ptr<Socket> Server::accept(Server::AcceptError& error) {
    assert(is_binded_);
    assert(is_listening_);

    SocketDescriptor client_socket_descriptor =
        ::accept(socket_descriptor_, nullptr, nullptr);
    if (client_socket_descriptor == kInvalidSocket) {
        if (errno == EINTR) {
            error = Server::AcceptError::kInterrupt;
        } else {
            error = Server::AcceptError::kUnknown;
        }

        return nullptr;
    }

    error = Server::AcceptError::kOk;
    return std::make_unique<Socket>(client_socket_descriptor);
}

static std::optional<SocketDescriptor> CreateNativeSocket() {
    SocketDescriptor socket_descriptor =
        ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_descriptor == kInvalidSocket) {
        return std::nullopt;
    }

    return socket_descriptor;
}

static void CloseNativeSocket(SocketDescriptor socket_descriptor) {
    ::close(socket_descriptor);
}

#endif

std::unique_ptr<Server> Server::createServer(Server::CreateError& error) {
    if (!IsSocketLibraryInitialized()) {
        error = Server::CreateError::kLibraryNotInitialized;
        return nullptr;
    }

    auto socket_descriptor = CreateNativeSocket();
    if (!socket_descriptor.has_value()) {
        error = Server::CreateError::kUnknown;
        return nullptr;
    }

    error = Server::CreateError::kOk;
    Server* server = new Server(socket_descriptor.value());
    return std::unique_ptr<Server>(server);
}

Server::~Server() { CloseNativeSocket(socket_descriptor_); }

}  // namespace simple_http
