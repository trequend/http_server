// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include "zero_message_body.h"

#include "message_body.h"
#include "socket_reader.h"

namespace simple_http {

size_t ZeroMessageBody::read(char* buffer, size_t length, ReadError& error) {
    error = ReadError::kOk;
    return 0;
}

MessageBody::ReadError ZeroMessageBody::consume() { return ReadError::kOk; }

}  // namespace simple_http
