// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include "http_headers.h"
#include "http_request_data.h"
#include "socket_writer.h"

namespace simple_http {

class OutgoingMessage {
   public:
    enum class WriteHeadError {
        kOk = 0,
        kConnectionClosed = 1,
        kAlreadySent = 2,
    };

    enum class WriteError {
        kOk = 0,
        kConnectionClosed = 1,
    };

    enum class EndError {
        kOk = 0,
        kConnectionClosed = 1,
    };

    enum class FlushError {
        kOk = 0,
        kConnectionClosed = 1,
    };

    OutgoingMessage() = delete;

    OutgoingMessage(const HttpRequestData& request_data, SocketWriter& output)
        : request_data_(request_data), output_(output){};

    HttpHeaders& getHeaders() { return headers_; };

    WriteHeadError writeHead(const std::string& code,
                             const std::string& message);

    WriteError write(const std::string& data);
    WriteError write(const char* buffer, size_t length);

    EndError end();

    FlushError flush();

    bool isStarted() { return is_head_sent_; };

    bool isEnded() { return is_ended_; }

   private:
    WriteError writeHeaders();

    const HttpRequestData& request_data_;
    SocketWriter& output_;

    HttpHeaders headers_;

    bool is_head_sent_ = false;
    bool is_ended_ = false;
};

}  // namespace simple_http
