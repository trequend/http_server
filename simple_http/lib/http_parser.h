// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include <optional>
#include <string_view>

#include "base_parser.h"

namespace simple_http {

class HttpParser : private BaseParser {
   public:
    struct RequestVersion {
        std::string_view major;
        std::string_view minor;
    };

    struct RequestLine {
        std::string_view method;
        std::string_view uri;
        std::optional<RequestVersion> version;
    };

    enum class ParseRequestLineError {
        kOk = 0,
        kMalformedMethod = 1,
        kMalformedUri = 2,
        kMalformedVersion = 3,
        kMalformedLine = 4,
    };

    struct RequestHeader {
        std::string_view name;
        std::string_view value;
    };

    enum class ParseRequestHeaderError {
        kOk = 0,
        kMalformedName = 1,
        kMalformedValue = 2,
        kMalformedLine = 4,
    };

    std::optional<RequestLine> parseRequestLine(const std::string_view& line,
                                                ParseRequestLineError& error);

    std::optional<RequestHeader> parseRequestHeader(
        const std::string_view& line, ParseRequestHeaderError& error);

   private:
    std::optional<std::string_view> parseMethod(const std::string_view& line,
                                                State& state);

    std::optional<std::string_view> parseUri(const std::string_view& line,
                                             State& state);

    std::optional<RequestVersion> parseVersion(const std::string_view& line,
                                               State& state);

    std::optional<std::string_view> parseNumber(const std::string_view& line,
                                                State& state);

    std::optional<std::string_view> parseHeaderName(
        const std::string_view& line, State& state);

    std::optional<std::string_view> parseHeaderValue(
        const std::string_view& line, State& state);

    std::optional<std::string_view> parseToken(const std::string_view& line,
                                               State& state);

    void skipSpaces(const std::string_view& line, State& state);

    void skipWhiteSpaces(const std::string_view& line, State& state);
};

}  // namespace simple_http
