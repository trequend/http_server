// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include <string_view>

namespace simple_http {

class BaseParser {
   protected:
    struct State {
        size_t index = 0;
        bool is_malformed = false;
    };

    bool parseSymbol(char symbol, const std::string_view& line, State& state);

    bool parseLiteral(const std::string_view& literal,
                      const std::string_view& line, State& state);
};

}  // namespace simple_http
