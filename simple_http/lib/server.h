// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include <chrono>
#include <memory>
#include <string>

#include "socket.h"
#include "socket_descriptor.h"

namespace simple_http {

class Server {
   public:
    enum class CreateError {
        kUnknown = -1,
        kOk = 0,
        kLibraryNotInitialized = 1,
    };

    struct BindOptions {
        std::string address = "127.0.0.1";
        int port = 3000;
    };

    enum class BindError {
        kUnknown = -1,
        kOk = 0,
        kWrongAddress = 1,
        kAddressInUse = 2,
        kNoAccess = 3,
    };

    struct ListenOptions {
        size_t backlog = 0;
    };

    enum class ListenError {
        kUnknown = -1,
        kOk = 0,
    };

    enum class AcceptError {
        kUnknown = -1,
        kOk = 0,
        kInterrupt = 1,
    };

    static std::unique_ptr<Server> createServer(CreateError& error);

    Server() = delete;

    ~Server();

    BindError bind(const BindOptions& options);

    ListenError listen(const ListenOptions& options);

    std::unique_ptr<Socket> accept(AcceptError& error);

   private:
    Server(SocketDescriptor socket_descriptor)
        : socket_descriptor_(socket_descriptor) {}

    SocketDescriptor socket_descriptor_;
    bool is_binded_ = false;
    bool is_listening_ = false;
};

}  // namespace simple_http
