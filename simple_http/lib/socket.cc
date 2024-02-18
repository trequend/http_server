// Copyright 2024 Balakin Dmitry. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include "socket.h"

#include <memory>
#include <string>

static bool g_IsSocketLibraryInitialized = false;

#ifdef _WIN32

#define _WINSOCKAPI_

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

simple_http::InitSocketLibraryError simple_http::InitSocketLibrary() {
    using namespace simple_http;

    if (g_IsSocketLibraryInitialized) {
        return InitSocketLibraryError::kAlreadyInitialzed;
    }

    WSADATA wsa_data;
    int result = ::WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        return InitSocketLibraryError::kUnknown;
    }

    g_IsSocketLibraryInitialized = true;
    return InitSocketLibraryError::kOk;
}

simple_http::CleanupSocketLibraryError simple_http::CleanupSocketLibrary() {
    using namespace simple_http;

    if (!g_IsSocketLibraryInitialized) {
        return CleanupSocketLibraryError::kOk;
    }

    int result = ::WSACleanup();
    if (result != 0) {
        return CleanupSocketLibraryError::kUnknown;
    }

    g_IsSocketLibraryInitialized = false;
    return CleanupSocketLibraryError::kOk;
}

simple_http::BindSocketError simple_http::Socket::bind(
    const BindSocketOptions& options) {
    using namespace simple_http;

    if (is_binded_) {
        return BindSocketError::kAlreadyBinded;
    }

    unsigned short port = static_cast<unsigned short>(options.port);
    if (options.port < 0 || options.port > 65535) {
        return BindSocketError::kWrongPort;
    }

    ::sockaddr_in socket_address;
    socket_address.sin_family = AF_INET;
    socket_address.sin_port = htons(port);

    int address_result =
        ::inet_pton(AF_INET, options.address.c_str(), &socket_address.sin_addr);
    if (address_result != 1) {
        return BindSocketError::kWrongAddress;
    }

    ::SOCKET native_socket = reinterpret_cast<::SOCKET>(native_socket_);
    int bind_result =
        ::bind(native_socket, reinterpret_cast<::sockaddr*>(&socket_address),
               sizeof(socket_address));
    if (bind_result == SOCKET_ERROR) {
        int error_code = ::WSAGetLastError();
        if (error_code == WSAEADDRINUSE) {
            return BindSocketError::kAddressInUse;
        } else if (error_code == WSAEACCES) {
            return BindSocketError::kNoAccess;
        }

        return BindSocketError::kUnknown;
    }

    is_binded_ = true;
    return BindSocketError::kOk;
}

simple_http::ListenSocketError simple_http::Socket::listen(
    const ListenSocketOptions& options) {
    using namespace simple_http;

    if (is_listening_) {
        return ListenSocketError::kAlreadyListening;
    }

    if (!is_binded_) {
        return ListenSocketError::kNotBinded;
    }

    if (options.max_connections < 1) {
        return ListenSocketError::kWrongMaxConnections;
    }

    ::SOCKET native_socket = reinterpret_cast<::SOCKET>(native_socket_);
    int listen_result = ::listen(native_socket, options.max_connections);
    if (listen_result == SOCKET_ERROR) {
        return ListenSocketError::kUnknown;
    }

    is_listening_ = true;
    return ListenSocketError::kOk;
}

void* simple_http::Socket::accept(simple_http::AcceptSocketError& error) {
    using namespace simple_http;
    if (!is_binded_) {
        error = AcceptSocketError::kNotBinded;
        return nullptr;
    }

    if (!is_listening_) {
        error = AcceptSocketError::kNotListining;
        return nullptr;
    }

    ::SOCKET native_socket = reinterpret_cast<::SOCKET>(native_socket_);
    ::SOCKET client_native_socket = ::accept(native_socket, nullptr, nullptr);
    if (client_native_socket == INVALID_SOCKET) {
        error = AcceptSocketError::kUnknown;
        return nullptr;
    }

    error = AcceptSocketError::kOk;
    return reinterpret_cast<void*>(client_native_socket);
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

std::unique_ptr<simple_http::Socket> simple_http::Socket::createSocket(
    simple_http::CreateSocketError& error) {
    using namespace simple_http;

    if (!g_IsSocketLibraryInitialized) {
        error = CreateSocketError::kLibraryNotInitialized;
        return nullptr;
    }

    void* native_socket = CreateNativeSocket();
    if (native_socket == nullptr) {
        error = CreateSocketError::kUnknown;
        return nullptr;
    }

    error = CreateSocketError::kOk;
    Socket* socket = new Socket(native_socket);
    return std::unique_ptr<Socket>(socket);
}

simple_http::Socket::~Socket() { CloseNativeSocket(native_socket_); }
