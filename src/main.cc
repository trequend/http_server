// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include <simple_http.h>
#include <winsock2.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>

#include "scope_guard.h"
#include "utils.h"

int main() {
    simple_http::InitLibraryError error = simple_http::InitLibrary();
    if (error != simple_http::InitLibraryError::kOk) {
        return EXIT_FAILURE;
    }

    ScopeGuard http_guard([]() { simple_http::CleanupLibrary(); });

    simple_http::Server::CreateError create_error;
    std::unique_ptr<simple_http::Server> server =
        simple_http::Server::createServer(create_error);
    if (create_error != simple_http::Server::CreateError::kOk) {
        std::cout << "Create error: " << static_cast<int>(create_error)
                  << std::endl;
        return EXIT_FAILURE;
    }

    simple_http::Server::BindOptions bind_options;
    bind_options.address = "127.0.0.1";  // localhost
    bind_options.port = 3000;
    simple_http::Server::BindError bind_error = server->bind(bind_options);
    if (bind_error != simple_http::Server::BindError::kOk) {
        std::cout << "Bind error: " << static_cast<int>(bind_error)
                  << std::endl;
        return EXIT_FAILURE;
    }

    simple_http::Server::ListenOptions listen_options;
    listen_options.backlog_size = 100;
    simple_http::Server::ListenError listen_error =
        server->listen(listen_options);
    if (listen_error != simple_http::Server::ListenError::kOk) {
        std::cout << "Listen error: " << static_cast<int>(listen_error)
                  << std::endl;
        return EXIT_FAILURE;
    }

    simple_http::HttpHeaders test_headers;
    test_headers.add("Content-Length", "126");
    test_headers.add("Content-Type", "text/html; charset=UTF-8");
    test_headers.add("X-Powered-By", "simple_http");
    test_headers.add("Set-Cookie", "hello=world!");
    test_headers.add("Set-Cookie", "session_data=data");
    std::cout << "=== Headers ===" << std::endl;
    PrintHeaders(test_headers);
    std::cout << "===============" << std::endl << std::endl;

    std::cout << "=== Parsers ===" << std::endl;

    std::string request_line =
        "GET      http://localhost:3000/hey%2f/how;are;you?world=hello "
        "HTTP/1.0";
    std::cout << "Request line: \"" << request_line << "\"" << std::endl;
    simple_http::HttpParser parser;
    simple_http::HttpParser::ParseRequestLineError parse_request_line_error;
    auto request_line_parse_result =
        parser.parseRequestLine(request_line, parse_request_line_error);
    PrintRequestLineParserResult(request_line_parse_result);

    if (request_line_parse_result.has_value()) {
        auto uri = request_line_parse_result.value().uri;
        simple_http::HttpUriParser uri_parser;
        auto uri_parse_result = uri_parser.parseUri(uri);
        PrintRequestUriParserResult(uri_parse_result);
    }

    std::cout << std::endl;

    std::string header_line = "Content-Length: 22";
    std::cout << "Header line: \"" << header_line << "\"" << std::endl;
    simple_http::HttpParser::ParseRequestHeaderError parse_header_error;
    auto request_header_parse_result =
        parser.parseRequestHeader(header_line, parse_header_error);
    PrintRequestHeaderParserResult(request_header_parse_result);

    std::cout << "===============" << std::endl << std::endl;

    std::cout << "Listening port 3000..." << std::endl;

    while (true) {
        simple_http::Server::AcceptError accept_error;
        std::unique_ptr<simple_http::Socket> client_socket =
            server->accept(accept_error);
        if (accept_error != simple_http::Server::AcceptError::kOk) {
            std::cout << "Accept error: " << static_cast<int>(accept_error)
                      << std::endl;
            continue;
        }

        std::cout << "Client connected" << std::endl;
        ScopeGuard socket_guard([&client_socket]() {
            client_socket->close();
            std::cout << "Client socket was closed" << std::endl;
        });

        std::chrono::milliseconds timeout(1000);  // 1 sec
        auto timeout_error = client_socket->setTimeout(timeout);
        if (timeout_error != simple_http::Socket::SetTimeoutError::kOk) {
            std::cout << "Set timeout error" << static_cast<int>(timeout_error)
                      << std::endl;
            continue;
        }

        char recv_buffer[4096];
        simple_http::SocketReader reader(client_socket.get(), recv_buffer,
                                         sizeof(recv_buffer));
        simple_http::SocketReader::ReadError read_error;
        simple_http::SocketReader::ReadResult request = reader.read(read_error);
        if (read_error != simple_http::SocketReader::ReadError::kOk) {
            std::cerr << "Read error: " << static_cast<int>(read_error)
                      << std::endl;
            continue;
        }

        printf("Request:\n======\n%.*s\n======\n",
               static_cast<int>(request.getLength()), request.getBuffer());

        char response_buffer[1024];
        simple_http::SocketWriter writer(client_socket.get(), response_buffer,
                                         sizeof(response_buffer));
        const char* http_line_parts[] = {"HTTP/1.0",
                                         " 200"
                                         " OK\r\n"};
        for (size_t i = 0; i < sizeof(http_line_parts) / sizeof(char*); i++) {
            auto write_error =
                writer.write(http_line_parts[i], strlen(http_line_parts[i]));
            if (write_error != simple_http::SocketWriter::WriteError::kOk) {
                std::cout << "Send error: " << static_cast<int>(write_error)
                          << std::endl;
            }
        }

        std::string file_name = "www/index.html";
        std::ifstream file(file_name, std::ios::binary);

        if (!file.is_open()) {
            std::cout << "File opening error" << std::endl;
            continue;
        }

        file.seekg(0, std::ios::end);
        std::streampos end_postion = file.tellg();
        size_t file_size = static_cast<size_t>(end_postion);
        file.seekg(0, std::ios::beg);

        simple_http::HttpHeaders headers;
        headers.add("Content-Length", std::to_string(file_size));
        headers.add("Content-Type", "text/html; charset=UTF-8");
        headers.add("X-Powered-By", "simple_http");

        for (auto headers_it = headers.begin(); headers_it != headers.end();
             headers_it++) {
            auto header_values = headers_it->second;
            for (auto header_it = header_values.begin();
                 header_it != header_values.end(); header_it++) {
                auto write_error = writer.write(headers_it->first);
                if (write_error != simple_http::SocketWriter::WriteError::kOk) {
                    std::cout << "Send error: " << static_cast<int>(write_error)
                              << std::endl;
                }

                write_error = writer.write(": ");
                if (write_error != simple_http::SocketWriter::WriteError::kOk) {
                    std::cout << "Send error: " << static_cast<int>(write_error)
                              << std::endl;
                }

                write_error = writer.write(*header_it);
                if (write_error != simple_http::SocketWriter::WriteError::kOk) {
                    std::cout << "Send error: " << static_cast<int>(write_error)
                              << std::endl;
                }

                write_error = writer.write("\r\n");
                if (write_error != simple_http::SocketWriter::WriteError::kOk) {
                    std::cout << "Send error: " << static_cast<int>(write_error)
                              << std::endl;
                }
            }
        }

        auto write_error = writer.write("\r\n");
        if (write_error != simple_http::SocketWriter::WriteError::kOk) {
            std::cout << "Send error: " << static_cast<int>(write_error)
                      << std::endl;
        }

        char read_buffer[1024];
        while (file.good()) {
            file.read(read_buffer, sizeof(read_buffer));
            size_t bytes_readed = file.gcount();
            if (bytes_readed > 0) {
                write_error = writer.write(read_buffer, bytes_readed);
                if (write_error != simple_http::SocketWriter::WriteError::kOk) {
                    std::cout << "Send error: " << static_cast<int>(write_error)
                              << std::endl;
                }
            }
        }

        if (file.bad() || !file.eof()) {
            std::cout << "File reading error" << std::endl;
        }

        file.close();

        writer.flush();
    }

    return EXIT_SUCCESS;
}
