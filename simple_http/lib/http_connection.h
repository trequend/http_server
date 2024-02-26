// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <memory>

#include "http_headers.h"
#include "http_method.h"
#include "http_parser.h"
#include "http_uri_parser.h"
#include "http_version.h"
#include "message_body.h"
#include "socket_reader.h"
#include "socket_writer.h"

namespace simple_http {
class HttpConnection {
   public:
    enum class ProccessRequestError {
        kUnknown = -1,
        kOk = 0,
        kAlreadyProcessed = 1,
    };

    HttpConnection() = delete;

    HttpConnection(Socket* socket, SocketReader& input, SocketWriter& output)
        : socket_(socket), input_(input), output_(output){};

    ProccessRequestError proccessRequest(std::function<void()> handler);

    HttpHeaders request_headers_;

    HttpMethod method_ = HttpMethod::kNone;

    std::string method_name_;

    HttpVersion http_version_ = HttpVersion::kNone;

    std::string uri_;

    std::string path_;

    std::string query_;

    std::unique_ptr<MessageBody> message_body_;

    size_t content_length_;

   private:
    enum class RequestProcessingState {
        kInitial,
        kRequestLine,
        kHeaders,
        kParsed,
    };

    enum class ParseRequestError {
        kUnknown = -1,
        kOk = 0,
    };

    enum class ParseError {
        kOk = 0,
        kBadRequest = 1,
        kLimitsExceeded = 2,
    };

    ParseError parseRequest(SocketReader::ReadResult result);

    ParseError takeRequestLine(SocketReader::ReadResult result);

    ParseError proccessRequestLine(HttpParser::RequestLine request_line);

    ParseError takeHeader(SocketReader::ReadResult result);

    ParseError proccessHeader(HttpParser::RequestHeader header);

    ParseError createMessageBody();

    void sendBadRequest();

    void sendInternalError();

    Socket* socket_;
    SocketReader& input_;
    SocketWriter& output_;

    HttpParser parser_;
    HttpUriParser uri_parser_;

    RequestProcessingState processing_state_ = RequestProcessingState::kInitial;
};
}  // namespace simple_http
