// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#ifdef _WIN32

#include <winsock2.h>

#endif

namespace simple_http {

#if _WIN32
typedef ::SOCKET SocketDescriptor;
#elif __linux__
typedef int SocketDescriptor;
#endif

}  // namespace simple_http
