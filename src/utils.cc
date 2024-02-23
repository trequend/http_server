// Copyright 2024 Balakin Dmitry. All rights reserved.
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
