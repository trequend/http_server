#include "socket_reader.h"

#include <algorithm>

#include "socket.h"

simple_http::SocketReader::ReadResult simple_http::SocketReader::read(
    simple_http::SocketReader::ReadError& error) {
    using namespace simple_http;

    Socket::ReadError read_error;
    size_t bytes_count =
        socket_->read(buffer_ + received_bytes_,
                      buffer_length_ - received_bytes_, read_error);
    if (read_error != Socket::ReadError::kOk) {
        error = SocketReader::ReadError::kUnknown;
        return SocketReader::ReadResult();
    }

    received_bytes_ += bytes_count;
    bool is_completed = bytes_count == 0;
    error = SocketReader::ReadError::kOk;
    return SocketReader::ReadResult(buffer_, received_bytes_, is_completed);
}

simple_http::SocketReader::AdvanceError simple_http::SocketReader::advance(
    size_t consumed_bytes) {
    using namespace simple_http;

    if (consumed_bytes < 0 || consumed_bytes > received_bytes_) {
        return SocketReader::AdvanceError::kOutOfBounds;
    }

    std::copy(buffer_ + consumed_bytes, buffer_ + received_bytes_, buffer_);
    return SocketReader::AdvanceError::kOk;
}
