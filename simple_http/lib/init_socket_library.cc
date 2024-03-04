// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include "init_socket_library.h"

#include <atomic>

#ifdef _WIN32

#define _WINSOCKAPI_

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

#endif

namespace simple_http {

static std::atomic<bool> g_IsSocketLibraryInitialized = false;

#ifdef _WIN32

InitSocketLibraryError InitSocketLibrary() {
    if (g_IsSocketLibraryInitialized) {
        return InitSocketLibraryError::kAlreadyInitialzed;
    }

    WSADATA wsa_data;
    int result = ::WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        return InitSocketLibraryError::kUnknown;
    }

    g_IsSocketLibraryInitialized = true;
    return InitSocketLibraryError::kOk;
}

CleanupSocketLibraryError CleanupSocketLibrary() {
    if (!g_IsSocketLibraryInitialized) {
        return CleanupSocketLibraryError::kOk;
    }

    int result = ::WSACleanup();
    if (result != 0) {
        return CleanupSocketLibraryError::kUnknown;
    }

    g_IsSocketLibraryInitialized = false;
    return CleanupSocketLibraryError::kOk;
}

#elif __linux__

InitSocketLibraryError InitSocketLibrary() {
    if (g_IsSocketLibraryInitialized) {
        return InitSocketLibraryError::kAlreadyInitialzed;
    }

    g_IsSocketLibraryInitialized = true;
    return InitSocketLibraryError::kOk;
}

CleanupSocketLibraryError CleanupSocketLibrary() {
    if (!g_IsSocketLibraryInitialized) {
        return CleanupSocketLibraryError::kOk;
    }

    g_IsSocketLibraryInitialized = false;
    return CleanupSocketLibraryError::kOk;
}

#endif

bool IsSocketLibraryInitialized() { return g_IsSocketLibraryInitialized; }

}  // namespace simple_http
