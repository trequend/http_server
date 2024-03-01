// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <memory>

#include "http_connection_handler.h"
#include "http_headers.h"
#include "http_method.h"
#include "http_parser.h"
#include "http_uri_parser.h"
#include "http_version.h"
#include "incoming_message.h"
#include "message_body.h"
#include "outgoing_message.h"
#include "socket_reader.h"
#include "socket_writer.h"

namespace simple_http {
class HttpConnection {
   public:
    enum class ProccessRequestError {
        kOk = 0,
        kConnectionClosed = 1,
        kBadSyntax = 2,
        kHandlerException = 3,
    };

    HttpConnection() = delete;

    HttpConnection(Socket* socket, SocketReader& input, SocketWriter& output)
        : socket_(socket), input_(input), output_(output){};

    ProccessRequestError proccessRequest(HttpConnectionHandler handler);

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

    ParseError takeMessageBody();

    void sendBadRequest();

    void sendInternalError();

    Socket* socket_;
    SocketReader& input_;
    SocketWriter& output_;

    HttpParser parser_;
    HttpUriParser uri_parser_;

    RequestProcessingState processing_state_ = RequestProcessingState::kInitial;

    HttpRequestData request_data_;
};
}  // namespace simple_http
