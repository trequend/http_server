// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include "http_uri_parser.h"

#include <charconv>
#include <optional>
#include <string_view>
#include <unordered_set>

namespace simple_http {

static const std::unordered_set<char> kSubDelims{'!', '$', '&', '\'', '(', ')',
                                                 '*', '+', ',', ';',  '='};

static const std::unordered_set<char> kUnreserved{'-', '.', '_', '~'};

static bool IsAlpha(char symbol) {
    return (symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z');
}

static bool IsDigit(char symbol) { return symbol >= '0' && symbol <= '9'; }

static bool IsSubDelims(char symbol) {
    return kSubDelims.find(symbol) != kSubDelims.end();
}

static bool IsUnreserved(char symbol) {
    return IsAlpha(symbol) || IsDigit(symbol) ||
           kUnreserved.find(symbol) != kUnreserved.end();
}

static bool IsHex(char symbol) {
    return IsDigit(symbol) || (symbol >= 'a' && symbol <= 'f') ||
           (symbol >= 'A' && symbol <= 'F');
}

std::optional<HttpUriParser::UriParts> HttpUriParser::parseUri(
    const std::string_view& line) {
    State state;

    auto absolute_uri_parts = parseAbsoluteUri(line, state);
    if (absolute_uri_parts.has_value()) {
        return absolute_uri_parts;
    } else if (state.is_malformed) {
        return std::nullopt;
    }

    UriParts uri_parts;
    auto absolute_path = parseAbsolutePath(line, state);
    if (!absolute_path.has_value()) {
        return std::nullopt;
    }
    uri_parts.path = absolute_path.value();

    auto query = parseQuery(line, state);
    if (state.is_malformed) {
        return std::nullopt;
    }
    if (query.has_value()) {
        uri_parts.query = query.value();
    }

    if (state.index != line.length()) {
        return std::nullopt;
    }

    return uri_parts;
}

std::optional<HttpUriParser::UriParts> HttpUriParser::parseAbsoluteUri(
    const std::string_view& line, State& state) {
    UriParts uri_parts;

    if (!parseLiteral("http://", line, state)) {
        return std::nullopt;
    }

    auto ipv4_address = parseIPv4Address(line, state);
    if (state.is_malformed) {
        return std::nullopt;
    }

    if (ipv4_address.has_value()) {
        uri_parts.host = ipv4_address.value();
    } else {
        auto hostname = parseHostname(line, state);
        if (!hostname.has_value()) {
            state.is_malformed = true;
            return std::nullopt;
        }
        uri_parts.host = hostname.value();
    }

    if (parseSymbol(':', line, state)) {
        auto port = parsePort(line, state);
        if (!port.has_value()) {
            state.is_malformed = true;
            return std::nullopt;
        }
        uri_parts.port = port.value();
    }

    auto absolute_path = parseAbsolutePath(line, state);
    if (state.is_malformed) {
        return std::nullopt;
    }

    if (absolute_path.has_value()) {
        uri_parts.path = absolute_path;
    }

    auto query = parseQuery(line, state);
    if (state.is_malformed) {
        return std::nullopt;
    }

    if (query.has_value()) {
        uri_parts.query = query.value();
    }

    if (state.index != line.length()) {
        state.is_malformed = true;
        return std::nullopt;
    }

    return uri_parts;
}

std::optional<std::string_view> HttpUriParser::parseIPv4Address(
    const std::string_view& line, State& state) {
    constexpr size_t kDecimalsCount = 4;

    if (state.index >= line.length() || !IsDigit(line[state.index])) {
        return std::nullopt;
    }

    size_t start = state.index;
    for (size_t i = 0; i < kDecimalsCount; i++) {
        auto decimal = parseDecimal(line, state);
        if (state.is_malformed) {
            return std::nullopt;
        }

        if (decimal.has_value()) {
            auto decimal_token = decimal.value();

            // Max: 255 - 3 symbols
            if (decimal_token.length() > 3) {
                state.is_malformed = true;
                return std::nullopt;
            }

            int value;
            std::from_chars(decimal_token.data(),
                            decimal_token.data() + decimal_token.length(),
                            value);
            if (value > 255) {
                state.is_malformed = true;
                return std::nullopt;
            }
        } else if (!parseSymbol('0', line, state)) {
            state.is_malformed = true;
            return std::nullopt;
        }

        if (i + 1 != kDecimalsCount && !parseSymbol('.', line, state)) {
            state.is_malformed = true;
            return std::nullopt;
        }
    }

    return std::string_view(line.data() + start, state.index - start);
}

std::optional<std::string_view> HttpUriParser::parseDecimal(
    const std::string_view& line, State& state) {
    size_t start = state.index;
    size_t index = state.index;
    while (index < line.length() && IsDigit(line[index]) &&
           (line[index] != '0' || start != index)) {
        index++;
    }

    if (index == start) {
        return std::nullopt;
    }

    state.index = index;
    return std::string_view(line.data() + start, index - start);
}

std::optional<std::string_view> HttpUriParser::parseHostname(
    const std::string_view& line, State& state) {
    size_t start = state.index;
    if (state.index >= line.length() || IsDigit(line[state.index])) {
        return std::nullopt;
    }

    while (state.index < line.length()) {
        if (IsUnreserved(line[state.index]) || IsSubDelims(line[state.index])) {
            state.index++;
        } else {
            if (!parseEncodedSymbol(line, state)) {
                break;
            }
        }
    }

    if (state.is_malformed) {
        return std::nullopt;
    }

    if (start == state.index) {
        return std::nullopt;
    }

    return std::string_view(line.data() + start, state.index - start);
}

std::optional<std::string_view> HttpUriParser::parsePort(
    const std::string_view& line, State& state) {
    size_t start = state.index;
    size_t index = state.index;
    size_t first_non_zero = -1;
    while (index < line.length() && IsDigit(line[index])) {
        if (line[index] != '0' && first_non_zero == -1) {
            first_non_zero = index;
        }

        index++;
    }

    state.index = index;

    if (index == start) {
        return std::nullopt;
    }

    std::string_view port_token;
    if (first_non_zero == -1) {
        size_t last = index - 1;
        port_token = std::string_view(line.data() + last, 1);
    } else {
        port_token = std::string_view(line.data() + first_non_zero,
                                      index - first_non_zero);
    }

    // Max: 65535 - 5 symbols
    if (port_token.length() > 5) {
        state.is_malformed = true;
        return std::nullopt;
    }

    int value;
    std::from_chars(port_token.data(), port_token.data() + port_token.length(),
                    value);
    if (value > 65535) {
        state.is_malformed = true;
        return std::nullopt;
    }

    return port_token;
}

std::optional<std::string_view> HttpUriParser::parseAbsolutePath(
    const std::string_view& line, State& state) {
    size_t start = state.index;
    while (parseSymbol('/', line, state)) {
        parseSegment(line, state);
        if (state.is_malformed) {
            return std::nullopt;
        }
    }

    if (start == state.index) {
        return std::nullopt;
    }

    return std::string_view(line.data() + start, state.index - start);
}

bool HttpUriParser::parseSegment(const std::string_view& line, State& state) {
    size_t start = state.index;
    while (state.index < line.length()) {
        auto symbol = line[state.index];
        if (IsUnreserved(symbol) || IsSubDelims(symbol) || symbol == ':' ||
            symbol == '@') {
            state.index++;
        } else {
            if (!parseEncodedSymbol(line, state)) {
                break;
            }
        }
    }

    if (state.is_malformed) {
        return false;
    }

    return state.index != start;
}

std::optional<std::string_view> HttpUriParser::parseQuery(
    const std::string_view& line, State& state) {
    if (!parseSymbol('?', line, state)) {
        return std::nullopt;
    }

    size_t start = state.index;
    while (state.index < line.length()) {
        auto symbol = line[state.index];
        if (IsUnreserved(symbol) || IsSubDelims(symbol) || symbol == ':' ||
            symbol == '@' || symbol == '/' || symbol == '?') {
            state.index++;
        } else {
            if (!parseEncodedSymbol(line, state)) {
                break;
            }
        }
    }

    if (state.is_malformed) {
        return std::nullopt;
    }

    return std::string_view(line.data() + start, state.index - start);
}

bool HttpUriParser::parseEncodedSymbol(const std::string_view& line,
                                       State& state) {
    if (!parseSymbol('%', line, state)) {
        return false;
    }

    if (line.length() - state.index < 2 || !IsHex(line[state.index]) ||
        !IsHex(line[state.index + 1])) {
        state.is_malformed = true;
        return false;
    }

    state.index += 2;
    return true;
}

}  // namespace simple_http
