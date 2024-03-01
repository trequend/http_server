// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include "message_body.h"
#include "socket_reader.h"

namespace simple_http {

class ZeroMessageBody : public MessageBody {
   public:
    size_t read(char* buffer, size_t length,
                MessageBody::ReadError& error) override;

    MessageBody::ReadError consume() override;
};

}  // namespace simple_http
