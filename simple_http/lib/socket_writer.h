// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include <string>

#include "socket.h"

namespace simple_http {

class SocketWriter {
   public:
    enum class WriteError {
        kOk = 0,
        kConnectionClosed = 1,
    };

    enum class FlushError {
        kOk = 0,
        kConnectionClosed = 1,
    };

    SocketWriter() = delete;

    SocketWriter(Socket* socket, char* buffer, size_t buffer_length)
        : socket_(socket), buffer_(buffer), buffer_length_(buffer_length){};

    WriteError write(const std::string& value);
    WriteError write(const char* source_buffer, size_t source_buffer_length);

    FlushError flush();

   private:
    Socket* socket_ = nullptr;
    char* buffer_ = nullptr;
    size_t buffer_length_ = 0;
    size_t saved_bytes_ = 0;
};

}  // namespace simple_http
