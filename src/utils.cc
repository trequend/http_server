// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include "utils.h"

#include <simple_http.h>

#include <iostream>

void PrintHeaders(const simple_http::HttpHeaders& headers) {
    for (auto headers_it = headers.begin(); headers_it != headers.end();
         headers_it++) {
        std::cout << headers_it->first << ": [" << std::endl;
        auto header_values = headers_it->second;
        for (auto header_it = header_values.begin();
             header_it != header_values.end(); header_it++) {
            std::cout << "  \"" << *header_it << "\"" << std::endl;
        }
        std::cout << "]" << std::endl;
    }
}

void PrintRequestLineParserResult(
    const std::optional<simple_http::HttpParser::RequestLine> result) {
    if (!result.has_value()) {
        std::cout << "Malformed request line" << std::endl;
        return;
    }

    auto method = result.value().method;
    auto uri = result.value().uri;

    std::cout << "Method: \"" << method << "\"" << std::endl;
    std::cout << "Uri: \"" << uri << "\"" << std::endl;

    if (result.value().version.has_value()) {
        auto major = result.value().version.value().major;
        auto minor = result.value().version.value().minor;

        std::cout << "Major: \"" << result.value().version.value().major << "\""
                  << std::endl;
        std::cout << "Minor: \"" << result.value().version.value().minor << "\""
                  << std::endl;
    } else {
        std::cout << "No version" << std::endl;
    }

    simple_http::HttpUriParser uri_parser;
    auto uri_parts = uri_parser.parseUri(uri);
}

void PrintRequestHeaderParserResult(
    const std::optional<simple_http::HttpParser::RequestHeader> result) {
    if (!result.has_value()) {
        std::cout << "Malformed header line" << std::endl;
    }

    std::cout << "Header name: \"" << result.value().name << "\"" << std::endl;
    std::cout << "Header value: \"" << result.value().value << "\""
              << std::endl;
}

void PrintRequestUriParserResult(
    const std::optional<simple_http::HttpUriParser::UriParts> result) {
    if (!result.has_value()) {
        std::cout << "Malformed Uri" << std::endl;
        return;
    }

    if (result.value().host.has_value()) {
        std::cout << "Host: \"" << result.value().host.value() << "\""
                  << std::endl;
    } else {
        std::cout << "No host" << std::endl;
    }

    if (result.value().port.has_value()) {
        std::cout << "Port: \"" << result.value().port.value() << "\""
                  << std::endl;
    } else {
        std::cout << "No port" << std::endl;
    }

    if (result.value().path.has_value()) {
        std::cout << "Path: \"" << result.value().path.value() << "\""
                  << std::endl;
    } else {
        std::cout << "No path" << std::endl;
    }

    if (result.value().query.has_value()) {
        std::cout << "Query: \"" << result.value().query.value() << "\""
                  << std::endl;
    } else {
        std::cout << "No query" << std::endl;
    }
}
