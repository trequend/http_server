// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include "socket_reader.h"

#include <algorithm>

#include "socket.h"

namespace simple_http {

SocketReader::ReadResult SocketReader::read(SocketReader::ReadError& error) {
    if (!is_examined_ || is_completed_) {
        return SocketReader::ReadResult(buffer_, received_bytes_,
                                        is_completed_);
    }

    Socket::ReadError read_error;
    size_t bytes_count =
        socket_->read(buffer_ + received_bytes_,
                      buffer_length_ - received_bytes_, read_error);
    if (read_error != Socket::ReadError::kOk) {
        socket_->close();
        error = SocketReader::ReadError::kUnknown;
        return SocketReader::ReadResult();
    }

    received_bytes_ += bytes_count;
    is_completed_ = bytes_count == 0;
    error = SocketReader::ReadError::kOk;
    return SocketReader::ReadResult(buffer_, received_bytes_, is_completed_);
}

SocketReader::AdvanceError SocketReader::advance(size_t consumed_bytes) {
    return advance(consumed_bytes, consumed_bytes);
}

SocketReader::AdvanceError SocketReader::advance(size_t consumed_bytes,
                                                 size_t examined_bytes) {
    if (consumed_bytes < 0 || consumed_bytes > received_bytes_) {
        return SocketReader::AdvanceError::kOutOfBounds;
    }

    if (examined_bytes < 0 || examined_bytes > received_bytes_) {
        return SocketReader::AdvanceError::kOutOfBounds;
    }

    if (consumed_bytes > examined_bytes) {
        return SocketReader::AdvanceError::kOutOfBounds;
    }

    std::copy(buffer_ + consumed_bytes, buffer_ + received_bytes_, buffer_);
    received_bytes_ -= consumed_bytes;
    is_examined_ = examined_bytes == received_bytes_;
    return SocketReader::AdvanceError::kOk;
}

}  // namespace simple_http
