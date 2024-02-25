// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include "init_library.h"

#include <atomic>

#include "init_socket_library.h"

namespace simple_http {

static std::atomic<bool> g_IsLibraryInitialized = false;

InitLibraryError InitLibrary() {
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

CleanupLibraryError CleanupLibrary() {
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

bool IsLibraryInitialized() { return g_IsLibraryInitialized; }

}  // namespace simple_http
