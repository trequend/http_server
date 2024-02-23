// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include "init_library.h"

#include <atomic>

#include "init_socket_library.h"

static std::atomic<bool> g_IsLibraryInitialized = false;

simple_http::InitLibraryError simple_http::InitLibrary() {
    using namespace simple_http;

    if (g_IsLibraryInitialized) {
        return InitLibraryError::kAlreadyInitialzed;
    }

    InitSocketLibraryError socketError = InitSocketLibrary();
    if (socketError != InitSocketLibraryError::kOk) {
        return InitLibraryError::kSocketLibrary;
    }

    g_IsLibraryInitialized = true;
    return InitLibraryError::kOk;
}

simple_http::CleanupLibraryError simple_http::CleanupLibrary() {
    using namespace simple_http;

    if (!g_IsLibraryInitialized) {
        return CleanupLibraryError::kOk;
    }

    CleanupSocketLibraryError socketCode = CleanupSocketLibrary();
    if (socketCode != CleanupSocketLibraryError::kOk) {
        return CleanupLibraryError::kSocketLibrary;
    }

    g_IsLibraryInitialized = false;
    return CleanupLibraryError::kOk;
}

bool simple_http::IsLibraryInitialized() { return g_IsLibraryInitialized; }
