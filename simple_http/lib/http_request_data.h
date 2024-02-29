// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include <memory>
#include <string>

#include "http_headers.h"
#include "http_method.h"
#include "http_version.h"
#include "message_body.h"

namespace simple_http {

struct HttpRequestData {
    HttpMethod method;
    std::string method_name;
    std::string href;
    std::string path;
    std::string query;
    HttpVersion http_version;
    HttpHeaders headers;
    size_t content_length;
    std::unique_ptr<MessageBody> body;
};

}  // namespace simple_http
