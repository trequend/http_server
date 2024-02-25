#include "http_parser.h"

#include <string>
#include <string_view>
#include <unordered_set>

namespace simple_http {

static constexpr char kSP = ' ';

static constexpr char kHT = '\t';

static const std::unordered_set<char> kTspecials{
    '(', ')', '<', '>', '@', ',', ';', ':', '\\', '"',
    '/', '[', ']', '?', '=', '{', '}', kSP, kHT};

static bool IsChar(char symbol) { return symbol >= 0 && symbol <= 127; }

static bool IsDigit(char symbol) { return symbol >= '0' && symbol <= '9'; }

static bool IsCTL(char symbol) {
    return (symbol >= 0 && symbol <= 31) || symbol == 127;
}

static bool IsTspecials(char symbol) {
    return kTspecials.find(symbol) != kTspecials.end();
}

std::optional<HttpParser::RequestLine> HttpParser::parseRequestLine(
    const std::string_view& line, HttpParser::ParseRequestLineError& error) {
    RequestLine request_line;
    State state;

    auto method = parseMethod(line, state);
    if (!method.has_value()) {
        error = ParseRequestLineError::kMalformedMethod;
        return std::nullopt;
    }
    request_line.method = method.value();

    if (!parseSymbol(kSP, line, state)) {
        error = ParseRequestLineError::kMalformedLine;
        return std::nullopt;
    }
    skipSpaces(line, state);

    auto uri = parseUri(line, state);
    if (!uri.has_value()) {
        error = ParseRequestLineError::kMalformedUri;
        return std::nullopt;
    }
    request_line.uri = uri.value();

    if (state.index == line.length()) {
        request_line.version = std::nullopt;
        error = ParseRequestLineError::kOk;
        return request_line;
    }

    if (!parseSymbol(kSP, line, state)) {
        error = ParseRequestLineError::kMalformedLine;
        return std::nullopt;
    }
    skipSpaces(line, state);

    auto version = parseVersion(line, state);
    if (!version.has_value()) {
        error = ParseRequestLineError::kMalformedVersion;
        return std::nullopt;
    }
    request_line.version = version.value();

    skipSpaces(line, state);
    if (state.index != line.length()) {
        error = ParseRequestLineError::kMalformedLine;
        return std::nullopt;
    }

    error = ParseRequestLineError::kOk;
    return request_line;
}

std::optional<HttpParser::RequestHeader> HttpParser::parseRequestHeader(
    const std::string_view& line, ParseRequestHeaderError& error) {
    State state;

    auto header_name = parseHeaderName(line, state);
    if (!header_name.has_value()) {
        error = ParseRequestHeaderError::kMalformedName;
        return std::nullopt;
    }

    if (!parseSymbol(':', line, state)) {
        error = ParseRequestHeaderError::kMalformedLine;
        return std::nullopt;
    }
    skipWhiteSpaces(line, state);

    auto header_value = parseHeaderValue(line, state);
    if (!header_value.has_value()) {
        error = ParseRequestHeaderError::kMalformedValue;
        return std::nullopt;
    }

    if (state.index != line.length()) {
        error = ParseRequestHeaderError::kMalformedLine;
        return std::nullopt;
    }

    RequestHeader header;
    header.name = header_name.value();
    header.value = header_value.value();
    return header;
}

std::optional<std::string_view> HttpParser::parseMethod(
    const std::string_view& line, State& state) {
    return parseToken(line, state);
}

std::optional<std::string_view> HttpParser::parseUri(
    const std::string_view& line, State& state) {
    size_t start = state.index;

    if (!parseSymbol('/', line, state) &&
        !parseLiteral("http://", line, state)) {
        return std::nullopt;
    }

    while (state.index < line.length() && line[state.index] != kSP) {
        state.index++;
    }

    return std::string_view(line.data() + start, state.index - start);
}

std::optional<HttpParser::RequestVersion> HttpParser::parseVersion(
    const std::string_view& line, State& state) {
    RequestVersion version;

    if (!parseLiteral("HTTP/", line, state)) {
        return std::nullopt;
    }

    auto major = parseNumber(line, state);
    if (!major.has_value()) {
        state.is_malformed = true;
        return std::nullopt;
    }
    version.major = major.value();

    if (!parseSymbol('.', line, state)) {
        state.is_malformed = true;
        return std::nullopt;
    }

    auto minor = parseNumber(line, state);
    if (!minor.has_value()) {
        state.is_malformed = true;
        return std::nullopt;
    }
    version.minor = minor.value();

    return version;
}

std::optional<std::string_view> HttpParser::parseNumber(
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

    if (first_non_zero == -1) {
        size_t last = index - 1;
        return std::string_view(line.data() + last, 1);
    }

    return std::string_view(line.data() + first_non_zero,
                            index - first_non_zero);
}

std::optional<std::string_view> HttpParser::parseHeaderName(
    const std::string_view& line, State& state) {
    return parseToken(line, state);
}

std::optional<std::string_view> HttpParser::parseHeaderValue(
    const std::string_view& line, State& state) {
    size_t start = state.index;
    size_t last_non_empty = -1;
    while (state.index < line.length()) {
        if (line[state.index] != kSP && line[state.index] != kHT) {
            last_non_empty = state.index;
        }

        state.index++;
    }

    if (last_non_empty == -1) {
        return std::string_view(line.data() + start, 0);
    }

    return std::string_view(line.data() + start, (last_non_empty + 1) - start);
}

std::optional<std::string_view> HttpParser::parseToken(
    const std::string_view& line, State& state) {
    size_t start = state.index;
    size_t index = state.index;
    while (index < line.length() && IsChar(line[index]) &&
           !IsCTL(line[index]) && !IsTspecials(line[index])) {
        index++;
    }

    state.index = index;

    if (index == start) {
        return std::nullopt;
    }

    return std::string_view(line.data() + start, index - start);
}

void HttpParser::skipSpaces(const std::string_view& line, State& state) {
    while (state.index < line.length() && line[state.index] == kSP) {
        state.index++;
    }
}

void HttpParser::skipWhiteSpaces(const std::string_view& line, State& state) {
    while (state.index < line.length() &&
           (line[state.index] == kSP || line[state.index] == kHT)) {
        state.index++;
    }
}

}  // namespace simple_http
