// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include "socket.h"

namespace simple_http {

class SocketReader {
   public:
    class ReadResult {
       public:
        ReadResult(){};

        ReadResult(const char* buffer, size_t length, bool is_completed)
            : buffer_(buffer), length_(length), is_completed_(is_completed){};

        const char* getBuffer() const { return buffer_; };

        bool isCompleted() const { return is_completed_; };

        size_t getLength() const { return length_; };

       private:
        const char* buffer_ = nullptr;
        size_t length_ = 0;
        bool is_completed_ = false;
    };

    enum class ReadError {
        kOk = 0,
        kConnectionClosed = 1,
    };

    enum class AdvanceError {
        kOk = 0,
        kOutOfBounds = 1,
    };

    SocketReader() = delete;

    SocketReader(Socket* socket, char* buffer, size_t buffer_length)
        : socket_(socket), buffer_(buffer), buffer_length_(buffer_length){};

    ReadResult read(ReadError& error);

    AdvanceError advance(size_t consumed_bytes, size_t examined_bytes);
    AdvanceError advance(size_t consumed_bytes);

   private:
    Socket* socket_ = nullptr;
    char* buffer_ = nullptr;
    size_t buffer_length_ = 0;

    bool is_completed_ = false;
    bool is_examined_ = true;
    size_t received_bytes_ = 0;
};

}  // namespace simple_http
