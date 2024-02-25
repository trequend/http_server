// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

namespace simple_http {

enum InitLibraryError {
    kOk = 0,
    kSocketLibrary = 1,
    kAlreadyInitialzed = 2,
};

InitLibraryError InitLibrary();

enum class CleanupLibraryError {
    kOk = 0,
    kSocketLibrary = -1,
};

CleanupLibraryError CleanupLibrary();

bool IsLibraryInitialized();

}  // namespace simple_http
