// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include "http_server.h"

#include <cassert>
#include <memory>

#include "http_connection.h"
#include "http_connection_handler.h"
#include "init_library.h"
#include "server.h"
#include "socket.h"
#include "thread_pool.h"

namespace simple_http {

HttpServer::~HttpServer() {
    if (should_cleanup_library_) {
        CleanupLibrary();
    }
}

std::unique_ptr<HttpServer> HttpServer::create(HttpConnectionHandler handler,
                                               HttpServer::CreateError& error) {
    return create(Options(), handler, error);
}

std::unique_ptr<HttpServer> HttpServer::create(HttpServer::Options options,
                                               HttpConnectionHandler handler,
                                               HttpServer::CreateError& error) {
    assert(options.timeout.count() >= 0);
    assert(options.request_buffer_length >= 1024);
    assert(options.response_buffer_length >= 1024);

    bool should_cleanup_library = false;
    if (!IsLibraryInitialized()) {
        should_cleanup_library = true;
        InitLibraryError library_error = InitLibrary();
        if (library_error != InitLibraryError::kOk) {
            error = CreateError::kLibraryNotInitialzed;
            return nullptr;
        }
    }

    std::unique_ptr<HttpServer> server =
        std::unique_ptr<HttpServer>(new HttpServer(options, handler));
    server->should_cleanup_library_ = should_cleanup_library;
    error = CreateError::kOk;
    return server;
}

HttpServer::ListenError HttpServer::listen(int port) {
    return listen(port, "127.0.0.1");
}

HttpServer::ListenError HttpServer::listen(int port, std::string hostname) {
    return listen(port, hostname, 100);
}

HttpServer::ListenError HttpServer::listen(int port, std::string hostname,
                                           size_t backlog) {
    Server::CreateError create_error;
    auto tcp_server = Server::createServer(create_error);
    if (create_error != Server::CreateError::kOk) {
        return HttpServer::ListenError::kUnknown;
    }

    Server::BindOptions bind_options;
    bind_options.port = port;
    bind_options.address = hostname;
    Server::BindError bind_error;
    bind_error = tcp_server->bind(bind_options);
    if (bind_error != Server::BindError::kOk) {
        switch (bind_error) {
            case Server::BindError::kWrongAddress:
                return ListenError::kWrongAddress;
            case Server::BindError::kAddressInUse:
                return ListenError::kAddressInUse;
            case Server::BindError::kNoAccess:
                return ListenError::kNoAccess;
            default:
                return ListenError::kUnknown;
        }
    }

    Server::ListenOptions listen_options;
    listen_options.backlog = backlog;
    Server::ListenError listen_error;
    listen_error = tcp_server->listen(listen_options);
    if (listen_error != Server::ListenError::kOk) {
        return ListenError::kUnknown;
    }

    auto thread_pool = ThreadPool<ThreadState>::create(
        options_.threads_count, [this](size_t index) {
            auto state = std::make_unique<ThreadState>();
            state->request_buffer.resize(options_.request_buffer_length);
            state->response_buffer.resize(options_.request_buffer_length);
            return state;
        });
    if (thread_pool == nullptr) {
        return ListenError::kPoolCreation;
    }

    while (true) {
        simple_http::Server::AcceptError accept_error;
        std::unique_ptr<simple_http::Socket> client_socket =
            tcp_server->accept(accept_error);
        if (accept_error != simple_http::Server::AcceptError::kOk) {
            continue;
        }

        if (options_.timeout.count() > 0) {
            Socket::SetTimeoutError set_timeout_error;
            set_timeout_error = client_socket->setTimeout(options_.timeout);
            if (set_timeout_error != Socket::SetTimeoutError::kOk) {
                continue;
            }
        }

        std::shared_ptr<simple_http::Socket> shared_client_socket =
            std::move(client_socket);
        thread_pool->post([client_socket = std::move(shared_client_socket),
                           this](ThreadState* state) {
            HttpConnection connection(client_socket.get(),
                                      state->request_buffer,
                                      state->response_buffer);
            connection.proccessRequest(handler_);
        });
    }

    return ListenError::kOk;
}

}  // namespace simple_http
