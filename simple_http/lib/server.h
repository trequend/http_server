// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <string>

#include "socket.h"

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
        std::chrono::milliseconds connection_timeout =
            std::chrono::milliseconds(0);
        int backlog_size = 0;
    };

    enum class ListenError {
        kUnknown = -1,
        kOk = 0,
    };

    enum class AcceptError {
        kUnknown = -1,
        kOk = 0,
    };

    static std::unique_ptr<Server> createServer(CreateError& error);

    Server() = delete;

    ~Server();

    BindError bind(const BindOptions& options);

    ListenError listen(const ListenOptions& options);

    std::unique_ptr<Socket> accept(AcceptError& error);

   private:
    Server(void* socket_descriptor) : socket_descriptor_(socket_descriptor) {}

    std::atomic<void*> socket_descriptor_ = {nullptr};
    std::atomic<bool> is_binded_ = {false};
    std::atomic<bool> is_listening_ = {false};
};

}  // namespace simple_http
