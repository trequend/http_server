// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include "http_connection.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <string_view>

#include "content_length_message_body.h"
#include "http_parser.h"
#include "http_uri_parser.h"
#include "incoming_message.h"
#include "outgoing_message.h"
#include "socket_reader.h"
#include "zero_message_body.h"

namespace simple_http {

static size_t FindCRLF(const char* buffer, size_t buffer_length) {
    for (size_t i = 0; i < buffer_length - 1; i++) {
        if (buffer[i] == '\r' && buffer[i + 1] == '\n') {
            return i;
        }
    }

    return -1;
}

static size_t IsEqualsCaseInsensitive(const std::string_view& first,
                                      const std::string_view& second) {
    if (first.length() != second.length()) {
        return false;
    }

    for (size_t i = 0; i < first.length(); i++) {
        if (::tolower(first[i]) != ::tolower(second[i])) {
            return false;
        }
    }

    return true;
}

HttpConnection::ProccessRequestError HttpConnection::proccessRequest(
    std::function<void(IncomingMessage request, OutgoingMessage response)>
        handler) {
    assert(processing_state_ == RequestProcessingState::kInitial);

    processing_state_ = RequestProcessingState::kRequestLine;
    do {
        SocketReader::ReadError read_error;
        SocketReader::ReadResult read_result = input_.read(read_error);
        if (read_error != SocketReader::ReadError::kOk) {
            return ProccessRequestError::kConnectionClosed;
        }

        ParseError parse_error = parseRequest(read_result);
        if (parse_error != ParseError::kOk) {
            sendBadRequest();
            return ProccessRequestError::kBadSyntax;
        }
    } while (processing_state_ != RequestProcessingState::kParsed);

    if (takeMessageBody() != ParseError::kOk) {
        sendBadRequest();
        return ProccessRequestError::kBadSyntax;
    }

    IncomingMessage request(request_data_);
    OutgoingMessage response(request_data_, output_);
    try {
        handler(request, response);
    } catch (...) {
        if (!response.isStarted()) {
            sendInternalError();
        }
        return ProccessRequestError::kHandlerException;
    }

    if (!response.isStarted()) {
        sendInternalError();
        return ProccessRequestError::kHandlerException;
    }

    if (!response.isEnded()) {
        socket_->close();
        return ProccessRequestError::kHandlerException;
    }

    if (socket_->isClosed()) {
        return ProccessRequestError::kConnectionClosed;
    }

    MessageBody::ReadError consume_error;
    consume_error = request_data_.body->consume();
    if (consume_error == MessageBody::ReadError::kBadSyntax) {
        socket_->close();
        return ProccessRequestError::kBadSyntax;
    }

    if (consume_error != MessageBody::ReadError::kOk) {
        return ProccessRequestError::kConnectionClosed;
    }

    socket_->close();
    return ProccessRequestError::kOk;
}

HttpConnection::ParseError HttpConnection::parseRequest(
    SocketReader::ReadResult result) {
    switch (processing_state_) {
        case RequestProcessingState::kRequestLine:
            return takeRequestLine(result);
        case RequestProcessingState::kHeaders:
            return takeHeader(result);
        case RequestProcessingState::kParsed:
            return ParseError::kOk;
    }

    return ParseError::kBadRequest;
}

HttpConnection::ParseError HttpConnection::takeRequestLine(
    SocketReader::ReadResult result) {
    size_t crlf_index = FindCRLF(result.getBuffer(), result.getLength());
    if (crlf_index == -1) {
        if (result.isCompleted()) {
            return ParseError::kBadRequest;
        }

        input_.advance(0, result.getLength());
        return ParseError::kOk;
    }

    std::string_view line(result.getBuffer(), crlf_index);
    HttpParser::ParseRequestLineError parse_error;
    auto parse_result = parser_.parseRequestLine(line, parse_error);
    if (!parse_result.has_value() ||
        parse_error != HttpParser::ParseRequestLineError::kOk) {
        return ParseError::kBadRequest;
    }

    HttpParser::RequestLine request_line = parse_result.value();
    ParseError error = proccessRequestLine(request_line);
    if (error != ParseError::kOk) {
        return error;
    }

    input_.advance(line.length() + 2);
    return ParseError::kOk;
}

HttpConnection::ParseError HttpConnection::proccessRequestLine(
    HttpParser::RequestLine request_line) {
    if (request_line.version.has_value()) {
        auto version = request_line.version.value();
        if (version.major.length() > 1 || version.minor.length() > 1) {
            return ParseError::kBadRequest;
        }

        if (version.major == "1" && version.minor == "0") {
            request_data_.http_version = HttpVersion::kHttp10;
        } else if (version.major == "1" && version.minor == "1") {
            request_data_.http_version = HttpVersion::kHttp11;
        } else {
            return ParseError::kBadRequest;
        }
        processing_state_ = RequestProcessingState::kHeaders;
    } else {
        processing_state_ = RequestProcessingState::kParsed;
        request_data_.http_version = HttpVersion::kHttp09;
    }

    if (request_data_.http_version == HttpVersion::kHttp09) {
        if (IsEqualsCaseInsensitive(request_line.method, "GET")) {
            request_data_.method = HttpMethod::kGet;
            request_data_.method_name = "GET";
        } else {
            return ParseError::kBadRequest;
        }
    } else {
        if (IsEqualsCaseInsensitive(request_line.method, "GET")) {
            request_data_.method = HttpMethod::kGet;
            request_data_.method_name = "GET";
        } else if (IsEqualsCaseInsensitive(request_line.method, "HEAD")) {
            request_data_.method = HttpMethod::kHead;
            request_data_.method_name = "HEAD";
        } else if (IsEqualsCaseInsensitive(request_line.method, "POST")) {
            request_data_.method = HttpMethod::kPost;
            request_data_.method_name = "POST";
        } else {
            request_data_.method = HttpMethod::kCustom;
            request_data_.method_name = request_line.method;
            std::transform(request_data_.method_name.begin(),
                           request_data_.method_name.end(),
                           request_data_.method_name.begin(), ::toupper);
        }
    }

    request_data_.href = request_line.uri;
    auto parse_result = uri_parser_.parseUri(request_line.uri);
    if (!parse_result.has_value()) {
        return ParseError::kBadRequest;
    }

    HttpUriParser::UriParts uri_parts = parse_result.value();
    if (uri_parts.path.has_value()) {
        request_data_.path = uri_parts.path.value();
    } else {
        request_data_.path = "/";
    }

    if (uri_parts.query.has_value()) {
        request_data_.query = uri_parts.query.value();
    } else {
        request_data_.query = "";
    }

    return ParseError::kOk;
}

HttpConnection::ParseError HttpConnection::takeHeader(
    SocketReader::ReadResult result) {
    size_t crlf_index = FindCRLF(result.getBuffer(), result.getLength());
    if (crlf_index == -1) {
        if (result.isCompleted()) {
            return ParseError::kBadRequest;
        }

        input_.advance(0, result.getLength());
        return ParseError::kOk;
    }

    if (crlf_index == 0) {
        input_.advance(2);
        processing_state_ = RequestProcessingState::kParsed;
        return ParseError::kOk;
    }

    std::string_view line(result.getBuffer(), crlf_index);
    HttpParser::ParseRequestHeaderError parse_error;
    auto parse_result = parser_.parseRequestHeader(line, parse_error);
    if (!parse_result.has_value() ||
        parse_error != HttpParser::ParseRequestHeaderError::kOk) {
        return ParseError::kBadRequest;
    }

    HttpParser::RequestHeader header = parse_result.value();
    ParseError error = proccessHeader(header);
    if (error != ParseError::kOk) {
        return error;
    }

    input_.advance(line.length() + 2);
    return ParseError::kOk;
}

HttpConnection::ParseError HttpConnection::proccessHeader(
    HttpParser::RequestHeader header) {
    std::string name(header.name);
    std::string value(header.value);
    request_data_.headers.add(std::move(name), std::move(value));
    return ParseError::kOk;
}

HttpConnection::ParseError HttpConnection::takeMessageBody() {
    if (request_data_.http_version == HttpVersion::kHttp09) {
        request_data_.body = std::make_unique<ZeroMessageBody>(input_);
        request_data_.content_length = 0;
        return ParseError::kOk;
    }

    auto headers_search_result = request_data_.headers.get("Content-Length");
    if (!headers_search_result.has_value()) {
        request_data_.body = std::make_unique<ZeroMessageBody>(input_);
        request_data_.content_length = 0;
        return ParseError::kOk;
    }

    auto headers = headers_search_result.value();
    if (headers.size() > 1) {
        return ParseError::kBadRequest;
    }

    const std::string& header = headers[0];
    if (header.length() > sizeof(size_t)) {
        return ParseError::kBadRequest;
    }

    for (size_t i = 0; i < header.length(); i++) {
        if (header[i] < '0' || header[i] > '9') {
            return ParseError::kBadRequest;
        }
    }

    size_t content_length = std::stoll(header);
    if (content_length < 0) {
        return ParseError::kBadRequest;
    }

    if (content_length > 0) {
        request_data_.body =
            std::make_unique<ContentLengthMessageBody>(input_, content_length);
        request_data_.content_length = content_length;
        return ParseError::kOk;
    }

    request_data_.body = std::make_unique<ZeroMessageBody>(input_);
    request_data_.content_length = 0;
    return ParseError::kOk;
}

void HttpConnection::sendBadRequest() {
    if (request_data_.http_version != HttpVersion::kNone &&
        request_data_.http_version != HttpVersion::kHttp09) {
        output_.write("HTTP/1.0 400 Bad Request\r\n\r\n");
        output_.flush();
    }

    socket_->close();
}

void HttpConnection::sendInternalError() {
    if (request_data_.http_version != HttpVersion::kNone &&
        request_data_.http_version != HttpVersion::kHttp09) {
        output_.write("HTTP/1.0 500 Internal Server Error\r\n\r\n");
        output_.flush();
    }

    socket_->close();
}

}  // namespace simple_http
