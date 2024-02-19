// Copyright 2024 Balakin Dmitry. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <string>

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
        kAlreadyBinded = 1,
        kWrongAddress = 2,
        kWrongPort = 3,
        kAddressInUse = 4,
        kNoAccess = 5,
    };

    struct ListenOptions {
        std::chrono::milliseconds connection_timeout =
            std::chrono::milliseconds(0);
        int backlog_size = 0;
    };

    enum class ListenError {
        kUnknown = -1,
        kOk = 0,
        kNotBinded = 1,
        kAlreadyListening = 2,
        kWrongConnectionTimeout = 3,
        kWrongBacklogSize = 4,
    };

    enum class AcceptError {
        kUnknown = -1,
        kOk = 0,
        kNotBinded = 1,
        kNotListining = 2,
    };

    static std::unique_ptr<Server> createServer(CreateError& error);

    Server() = delete;

    ~Server();

    BindError bind(const BindOptions& options);

    ListenError listen(const ListenOptions& options);

    void* accept(AcceptError& error);

   private:
    Server(void* socket_descriptor) : socket_descriptor_(socket_descriptor) {}

    std::atomic<void*> socket_descriptor_ = nullptr;
    std::atomic<bool> is_binded_ = false;
    std::atomic<bool> is_listening_ = false;
};
}  // namespace simple_http
