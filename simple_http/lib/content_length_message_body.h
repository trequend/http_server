// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include "message_body.h"
#include "socket_reader.h"

namespace simple_http {

class ContentLengthMessageBody : public MessageBody {
   public:
    ContentLengthMessageBody() = delete;

    ContentLengthMessageBody(SocketReader& input, size_t length)
        : input_(input), remaining_bytes_(length) {}

    size_t read(char* buffer, size_t length,
                MessageBody::ReadError& error) override;

    MessageBody::ReadError consume() override;

   private:
    SocketReader& input_;
    size_t remaining_bytes_;
};

}  // namespace simple_http
