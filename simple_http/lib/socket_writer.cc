// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include "socket_writer.h"

#include <algorithm>
#include <cassert>
#include <string>

#include "socket.h"

#undef min

namespace simple_http {

SocketWriter::WriteError SocketWriter::write(const std::string& value) {
    return write(value.c_str(), value.length());
}

SocketWriter::WriteError SocketWriter::write(const char* source_buffer,
                                             size_t source_buffer_length) {
    assert(source_buffer_length >= 0);

    do {
        size_t bytes_to_copy =
            std::min(buffer_length_ - saved_bytes_, source_buffer_length);
        const char* source_start = source_buffer;
        const char* source_end = source_buffer + bytes_to_copy;
        char* destination_start = buffer_ + saved_bytes_;

        std::copy(source_start, source_end, destination_start);

        saved_bytes_ += bytes_to_copy;
        source_buffer_length -= bytes_to_copy;
        source_buffer += bytes_to_copy;

        if (saved_bytes_ == buffer_length_) {
            SocketWriter::FlushError flush_error = flush();
            if (flush_error != SocketWriter::FlushError::kOk) {
                return SocketWriter::WriteError::kConnectionClosed;
            }
        }
    } while (source_buffer_length != 0);

    return SocketWriter::WriteError::kOk;
}

SocketWriter::FlushError SocketWriter::flush() {
    if (saved_bytes_ == 0) {
        return SocketWriter::FlushError::kOk;
    }

    Socket::SendError send_error = socket_->send(buffer_, saved_bytes_);
    if (send_error != Socket::SendError::kOk) {
        socket_->close();
        return SocketWriter::FlushError::kConnectionClosed;
    }

    saved_bytes_ = 0;
    return SocketWriter::FlushError::kOk;
}

}  // namespace simple_http
