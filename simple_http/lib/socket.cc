// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include "socket.h"

#include <cassert>
#include <chrono>

#include "socket_descriptor.h"

#ifdef _WIN32

#include <winsock2.h>

#elif __linux__

#include <errno.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#endif

namespace simple_http {

#ifdef _WIN32

size_t Socket::read(char* buffer, size_t buffer_length,
                    Socket::ReadError& error) {
    int request_bytes_count =
        ::recv(socket_descriptor_, buffer, static_cast<int>(buffer_length), 0);
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

Socket::SendError Socket::send(const char* data, size_t length) {
    int result = ::send(socket_descriptor_, data, static_cast<int>(length), 0);
    if (result == SOCKET_ERROR) {
        int inner_error = ::WSAGetLastError();
        if (inner_error == WSAETIMEDOUT) {
            return Socket::SendError::kTimeout;
        }

        return Socket::SendError::kUnknown;
    }

    return Socket::SendError::kOk;
}

Socket::SetTimeoutError Socket::setTimeout(std::chrono::milliseconds timeout) {
    assert(timeout.count() >= 0);

    unsigned long milliseconds = static_cast<unsigned long>(timeout.count());
    if (::setsockopt(socket_descriptor_, SOL_SOCKET, SO_RCVTIMEO,
                     reinterpret_cast<const char*>(&timeout),
                     sizeof(timeout)) == SOCKET_ERROR ||
        ::setsockopt(socket_descriptor_, SOL_SOCKET, SO_SNDTIMEO,
                     reinterpret_cast<const char*>(&timeout),
                     sizeof(timeout)) == SOCKET_ERROR) {
        return Socket::SetTimeoutError::kConnectionClosed;
    }

    return Socket::SetTimeoutError::kOk;
}

static void CloseNativeSocket(SocketDescriptor socket_descriptor) {
    ::closesocket(socket_descriptor);
}

#elif __linux__

constexpr int kInvalidSocket = -1;

size_t Socket::read(char* buffer, size_t buffer_length,
                    Socket::ReadError& error) {
    int request_bytes_count =
        ::recv(socket_descriptor_, buffer, static_cast<int>(buffer_length), 0);
    if (request_bytes_count == kInvalidSocket) {
        if (errno == ETIMEDOUT) {
            error = Socket::ReadError::kTimeout;
        } else {
            error = Socket::ReadError::kUnknown;
        }

        return 0;
    }

    error = Socket::ReadError::kOk;
    return static_cast<size_t>(request_bytes_count);
}

Socket::SendError Socket::send(const char* data, size_t length) {
    int result = ::send(socket_descriptor_, data, static_cast<int>(length), 0);
    if (result == kInvalidSocket) {
        if (errno == ETIMEDOUT) {
            return Socket::SendError::kTimeout;
        }

        return Socket::SendError::kUnknown;
    }

    return Socket::SendError::kOk;
}

Socket::SetTimeoutError Socket::setTimeout(std::chrono::milliseconds timeout) {
    assert(timeout.count() >= 0);

    timeval native_timeout;
    {
        using namespace std::chrono;
        native_timeout.tv_sec = duration_cast<seconds>(timeout).count();
        native_timeout.tv_usec =
            duration_cast<microseconds>(timeout % seconds(1)).count();
    }
    if (::setsockopt(socket_descriptor_, SOL_SOCKET, SO_RCVTIMEO,
                     reinterpret_cast<const char*>(&native_timeout),
                     sizeof(native_timeout)) == kInvalidSocket ||
        ::setsockopt(socket_descriptor_, SOL_SOCKET, SO_SNDTIMEO,
                     reinterpret_cast<const char*>(&native_timeout),
                     sizeof(native_timeout)) == kInvalidSocket) {
        return Socket::SetTimeoutError::kConnectionClosed;
    }

    return Socket::SetTimeoutError::kOk;
}

static void CloseNativeSocket(SocketDescriptor socket_descriptor) {
    ::close(socket_descriptor);
}

#endif

void Socket::close() {
    if (is_closed_) {
        return;
    }

    CloseNativeSocket(socket_descriptor_);
    is_closed_ = true;
}

}  // namespace simple_http
