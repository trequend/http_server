// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include "http_server.h"

#include <iostream>

namespace simple_http {

void HttpServer::sayHello() { std::cout << "Hello world!" << std::endl; }

}  // namespace simple_http
