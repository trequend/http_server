// Copyright 2024 Balakin Dmitry. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include <chrono>

namespace simple_http {
class Socket {
   public:
    enum class ReadError {
        kUnknown = -1,
        kOk = 0,
        kTimeout = 1,
        kClosed = 2,
    };

    enum class SendError {
        kUnknown = -1,
        kOk = 0,
        kTimeout = 1,
        kClosed = 2,
    };

    enum class SetTimeoutError {
        kUnknown = -1,
        kOk = 0,
        kWrongTimeout = 1,
        kClosed = 2,
    };

    enum class CloseError {
        kUnknown = -1,
        kOk = 0,
    };

    Socket() = delete;
    Socket(void* socket_descriptor) : socket_descriptor_(socket_descriptor) {}

    ~Socket();

    size_t read(char* buffer, size_t buffer_length, ReadError& error);

    SendError send(const char* data, size_t length);

    SetTimeoutError setTimeout(std::chrono::milliseconds timeout);

    CloseError close();

   private:
    void* socket_descriptor_ = nullptr;
    bool is_closed_ = false;
};
}  // namespace simple_http
