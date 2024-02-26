// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include "zero_message_body.h"

#include "message_body.h"
#include "socket_reader.h"

namespace simple_http {

size_t ZeroMessageBody::read(char* buffer, size_t length, ReadError& error) {
    SocketReader::ReadError read_error;
    SocketReader::ReadResult result = input_.read(read_error);
    if (read_error != SocketReader::ReadError::kOk) {
        error = ReadError::kConnectionClosed;
        return -1;
    }

    if (result.getLength() != 0) {
        error = ReadError::kBadSyntax;
        return -1;
    }

    error = ReadError::kOk;
    return 0;
}

MessageBody::ReadError ZeroMessageBody::consume() {
    MessageBody::ReadError read_error;
    read(nullptr, 0, read_error);
    return read_error;
}

}  // namespace simple_http
