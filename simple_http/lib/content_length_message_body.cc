// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include "content_length_message_body.h"

#include <algorithm>

#include "message_body.h"

namespace simple_http {

size_t ContentLengthMessageBody::read(char *buffer, size_t length,
                                      MessageBody::ReadError &error) {
    size_t offset = 0;
    while (length != 0 && remaining_bytes_ != 0) {
        SocketReader::ReadError read_error;
        SocketReader::ReadResult result = input_.read(read_error);
        if (read_error != SocketReader::ReadError::kOk) {
            error = ReadError::kConnectionClosed;
            return -1;
        }

        if (result.getLength() > remaining_bytes_) {
            error = ReadError::kBadSyntax;
            return -1;
        }

        if (result.isCompleted() && result.getLength() < remaining_bytes_) {
            error = ReadError::kBadSyntax;
            return -1;
        }

        if (result.isCompleted() && result.getLength() == 0) {
            break;
        }

        size_t consumed_bytes = std::min(result.getLength(), length);
        const char *start = result.getBuffer();
        const char *end = result.getBuffer() + consumed_bytes;
        char *destination = buffer + offset;
        std::copy(start, end, destination);

        input_.advance(consumed_bytes);
        length -= consumed_bytes;
        remaining_bytes_ -= consumed_bytes;
        offset += consumed_bytes;
    };

    error = ReadError::kOk;
    return offset;
}

MessageBody::ReadError ContentLengthMessageBody::consume() {
    while (remaining_bytes_ != 0) {
        SocketReader::ReadError read_error;
        SocketReader::ReadResult result = input_.read(read_error);
        if (read_error != SocketReader::ReadError::kOk) {
            return ReadError::kConnectionClosed;
        }

        if (result.getLength() > remaining_bytes_) {
            return ReadError::kBadSyntax;
        }

        if (result.isCompleted() && result.getLength() < remaining_bytes_) {
            return ReadError::kBadSyntax;
        }

        remaining_bytes_ -= result.getLength();
        input_.advance(result.getLength());
    };

    return ReadError::kOk;
}

}  // namespace simple_http
