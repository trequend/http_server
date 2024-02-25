// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include <optional>
#include <string_view>

#include "base_parser.h"

namespace simple_http {
class HttpUriParser : private BaseParser {
   public:
    struct UriParts {
        std::optional<std::string_view> host;
        std::optional<std::string_view> port;
        std::optional<std::string_view> path;
        std::optional<std::string_view> query;
    };

    std::optional<UriParts> parseUri(const std::string_view& line);

   private:
    std::optional<UriParts> parseAbsoluteUri(const std::string_view& line,
                                             State& state);

    std::optional<std::string_view> parseIPv4Address(
        const std::string_view& line, State& state);

    std::optional<std::string_view> parseDecimal(const std::string_view& line,
                                                 State& state);

    std::optional<std::string_view> parseHostname(const std::string_view& line,
                                                  State& state);

    std::optional<std::string_view> parsePort(const std::string_view& line,
                                              State& state);

    std::optional<std::string_view> parseAbsolutePath(
        const std::string_view& line, State& state);

    bool parseSegment(const std::string_view& line, State& state);

    std::optional<std::string_view> parseQuery(const std::string_view& line,
                                               State& state);

    bool parseEncodedSymbol(const std::string_view& line, State& state);
};
}  // namespace simple_http
