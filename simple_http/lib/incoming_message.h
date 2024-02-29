// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include <string>

#include "http_request_data.h"
#include "message_body.h"

namespace simple_http {

class IncomingMessage {
   public:
    IncomingMessage() = delete;

    IncomingMessage(const HttpRequestData& data) : data_(data){};

    HttpMethod getMethod() const { return data_.method; };

    const std::string& getMethodName() const { return data_.method_name; };

    const std::string& getHref() const { return data_.href; };

    const std::string& getPath() const { return data_.path; };

    const std::string& getQuery() const { return data_.query; };

    HttpVersion getHttpVersion() const { return data_.http_version; };

    const HttpHeaders& getHeaders() const { return data_.headers; };

    size_t getContentLength() const { return data_.content_length; };

    size_t readBody(char* buffer, size_t length, MessageBody::ReadError& error);

   private:
    const HttpRequestData& data_;
};

}  // namespace simple_http
