// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include <string>

namespace simple_http {

class MessageBody {
   public:
    enum class ReadError {
        kOk = 0,
        kConnectionClosed = 1,
        kBadSyntax = 2,
    };

    virtual size_t read(char* buffer, size_t length, ReadError& error) = 0;

    virtual ReadError consume() = 0;
};

}  // namespace simple_http
