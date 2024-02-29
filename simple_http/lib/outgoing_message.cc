// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include "outgoing_message.h"

#include "http_version.h"

namespace simple_http {
OutgoingMessage::WriteHeadError OutgoingMessage::writeHead(
    const std::string& code, const std::string& message) {
    if (is_head_sent_) {
        return WriteHeadError::kAlreadySent;
    }

    is_head_sent_ = true;

    if (request_data_.http_version == HttpVersion::kHttp09) {
        return WriteHeadError::kOk;
    }

    std::string response_line = "HTTP/1.0 " + code + " " + message + "\r\n";
    SocketWriter::WriteError write_error;
    write_error = output_.write(response_line);
    if (write_error != SocketWriter::WriteError::kOk) {
        return WriteHeadError::kConnectionClosed;
    }

    WriteError write_headers_error;
    write_headers_error = writeHeaders();
    return write_headers_error == WriteError::kOk
               ? WriteHeadError::kOk
               : WriteHeadError::kConnectionClosed;
}

OutgoingMessage::WriteError OutgoingMessage::write(const std::string& data) {
    return write(data.c_str(), data.length());
}

OutgoingMessage::WriteError OutgoingMessage::write(const char* buffer,
                                                   size_t length) {
    if (!is_head_sent_) {
        WriteHeadError write_head_error;
        write_head_error = writeHead("200", "OK");
        if (write_head_error != WriteHeadError::kOk) {
            return WriteError::kConnectionClosed;
        }
    }

    SocketWriter::WriteError write_error;
    write_error = output_.write(buffer, length);
    return write_error == SocketWriter::WriteError::kOk
               ? WriteError::kOk
               : WriteError::kConnectionClosed;
}

OutgoingMessage::EndError OutgoingMessage::end() {
    if (is_ended_) {
        return EndError::kOk;
    }

    is_ended_ = true;

    FlushError flush_error;
    flush_error = flush();
    return flush_error == FlushError::kOk ? EndError::kOk
                                          : EndError::kConnectionClosed;
}

OutgoingMessage::FlushError OutgoingMessage::flush() {
    SocketWriter::FlushError flush_error;
    flush_error = output_.flush();
    return flush_error == SocketWriter::FlushError::kOk
               ? FlushError::kOk
               : FlushError::kConnectionClosed;
}

OutgoingMessage::WriteError OutgoingMessage::writeHeaders() {
    SocketWriter::WriteError write_error;
    for (auto it = headers_.begin(); it != headers_.end(); it++) {
        auto& name = it->first;
        auto& header_values = it->second;
        for (auto value_it = header_values.begin();
             value_it != header_values.end(); value_it++) {
            write_error = output_.write(name);
            if (write_error != simple_http::SocketWriter::WriteError::kOk) {
                return WriteError::kConnectionClosed;
            }

            write_error = output_.write(": ");
            if (write_error != simple_http::SocketWriter::WriteError::kOk) {
                return WriteError::kConnectionClosed;
            }

            write_error = output_.write(*value_it);
            if (write_error != simple_http::SocketWriter::WriteError::kOk) {
                return WriteError::kConnectionClosed;
            }

            write_error = output_.write("\r\n");
            if (write_error != simple_http::SocketWriter::WriteError::kOk) {
                return WriteError::kConnectionClosed;
            }
        }
    }

    write_error = output_.write("\r\n");
    if (write_error != simple_http::SocketWriter::WriteError::kOk) {
        return WriteError::kConnectionClosed;
    }

    return WriteError::kOk;
}

}  // namespace simple_http
