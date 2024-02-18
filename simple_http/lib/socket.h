// Copyright 2024 Balakin Dmitry. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include <memory>
#include <string>

namespace simple_http {
enum class InitSocketLibraryError {
    kUnknown = -1,
    kOk = 0,
    kAlreadyInitialzed = 1,
};

InitSocketLibraryError InitSocketLibrary();

enum class CleanupSocketLibraryError {
    kUnknown = -1,
    kOk = 0,
};

CleanupSocketLibraryError CleanupSocketLibrary();

enum class CreateSocketError {
    kUnknown = -1,
    kOk = 0,
    kLibraryNotInitialized = 1,
};

struct BindSocketOptions {
    std::string address;
    int port;
};

enum class BindSocketError {
    kUnknown = -1,
    kOk = 0,
    kAlreadyBinded = 1,
    kWrongAddress = 2,
    kWrongPort = 3,
    kAddressInUse = 4,
    kNoAccess = 5,
};

struct ListenSocketOptions {
    int max_connections;
};

enum class ListenSocketError {
    kUnknown = -1,
    kOk = 0,
    kNotBinded = 1,
    kAlreadyListening = 2,
    kWrongMaxConnections = 3,
};

enum class AcceptSocketError {
    kUnknown = -1,
    kOk = 0,
    kNotBinded = 1,
    kNotListining = 2,
};

class Socket {
   public:
    static std::unique_ptr<Socket> createSocket(CreateSocketError& error);

    Socket() = delete;

    ~Socket();

    BindSocketError bind(const BindSocketOptions& options);

    ListenSocketError listen(const ListenSocketOptions& options);

    void* accept(AcceptSocketError& error);

   private:
    Socket(void* native_socket) : native_socket_(native_socket) {}

    void* native_socket_ = nullptr;
    bool is_binded_ = false;
    bool is_listening_ = false;
};
}  // namespace simple_http