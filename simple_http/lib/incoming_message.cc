// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include "incoming_message.h"

#include "message_body.h"

namespace simple_http {

size_t IncomingMessage::readBody(char* buffer, size_t length,
                                 MessageBody::ReadError& error) {
    return data_.body->read(buffer, length, error);
}

}  // namespace simple_http
