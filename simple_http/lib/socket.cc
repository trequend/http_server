// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include "socket.h"

#include <chrono>

#ifdef _WIN32

#include <winsock2.h>

simple_http::Socket::~Socket() { close(); }

size_t simple_http::Socket::read(char* buffer, size_t buffer_length,
                                 simple_http::Socket::ReadError& error) {
    using namespace simple_http;

    ::SOCKET native_socket = reinterpret_cast<::SOCKET>(socket_descriptor_);
    int request_bytes_count =
        ::recv(native_socket, buffer, static_cast<int>(buffer_length), 0);
    if (request_bytes_count == SOCKET_ERROR) {
        int inner_error = ::WSAGetLastError();
        if (inner_error == WSAETIMEDOUT) {
            error = Socket::ReadError::kTimeout;
        } else {
            error = Socket::ReadError::kUnknown;
        }

        return 0;
    }

    error = Socket::ReadError::kOk;
    return static_cast<size_t>(request_bytes_count);
}

simple_http::Socket::SendError simple_http::Socket::send(const char* data,
                                                         size_t length) {
    using namespace simple_http;

    ::SOCKET native_socket = reinterpret_cast<::SOCKET>(socket_descriptor_);
    int result = ::send(native_socket, data, static_cast<int>(length), 0);
    if (result == SOCKET_ERROR) {
        int inner_error = ::WSAGetLastError();
        if (inner_error == WSAETIMEDOUT) {
            return Socket::SendError::kTimeout;
        }

        return Socket::SendError::kUnknown;
    }

    return Socket::SendError::kOk;
}

simple_http::Socket::SetTimeoutError simple_http::Socket::setTimeout(
    std::chrono::milliseconds timeout) {
    using namespace simple_http;

    if (timeout.count() < 0) {
        return Socket::SetTimeoutError::kWrongTimeout;
    }

    ::SOCKET native_socket = reinterpret_cast<::SOCKET>(socket_descriptor_);
    unsigned long milliseconds = static_cast<unsigned long>(timeout.count());
    if (setsockopt(native_socket, SOL_SOCKET, SO_RCVTIMEO,
                   reinterpret_cast<const char*>(&timeout),
                   sizeof(timeout)) == SOCKET_ERROR ||
        setsockopt(native_socket, SOL_SOCKET, SO_SNDTIMEO,
                   reinterpret_cast<const char*>(&timeout),
                   sizeof(timeout)) == SOCKET_ERROR) {
        return Socket::SetTimeoutError::kUnknown;
    }

    return Socket::SetTimeoutError::kOk;
}

simple_http::Socket::CloseError simple_http::Socket::close() {
    using namespace simple_http;

    if (is_closed_) {
        return Socket::CloseError::kOk;
    }

    ::SOCKET native_socket = reinterpret_cast<::SOCKET>(socket_descriptor_);
    int result = ::closesocket(native_socket);
    if (result == SOCKET_ERROR) {
        return Socket::CloseError::kUnknown;
    }

    is_closed_ = true;
    return Socket::CloseError::kOk;
}
#endif
