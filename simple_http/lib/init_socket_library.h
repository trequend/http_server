// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

namespace simple_http {

enum class InitSocketLibraryError {
    kUnknown = -1,
    kOk = 0,
    kAlreadyInitialzed = 1,
};

InitSocketLibraryError InitSocketLibrary();

enum class CleanupSocketLibraryError {
    kUnknown = -1,
    kOk = 0,
};

CleanupSocketLibraryError CleanupSocketLibrary();

bool IsSocketLibraryInitialized();

}  // namespace simple_http
