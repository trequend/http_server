// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include <simple_http.h>

void PrintHeaders(const simple_http::HttpHeaders& headers);

void PrintRequestLineParserResult(
    const std::optional<simple_http::HttpParser::RequestLine> result);

void PrintRequestHeaderParserResult(
    const std::optional<simple_http::HttpParser::RequestHeader> result);

void PrintRequestUriParserResult(
    const std::optional<simple_http::HttpUriParser::UriParts> result);
