// Copyright 2024 Balakin Dmitry. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include <functional>

class ScopeGuard {
   public:
    ScopeGuard(std::function<void()> onExit) : on_exit_(std::move(onExit)) {}

    ~ScopeGuard() { on_exit_(); }

   private:
    std::function<void()> on_exit_;
};