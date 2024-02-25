#include "base_parser.h"

#include <cctype>
#include <string_view>

namespace simple_http {
bool BaseParser::parseSymbol(char symbol, const std::string_view& line,
                             State& state) {
    std::string_view literal(&symbol, 1);
    return parseLiteral(literal, line, state);
}

bool BaseParser::parseLiteral(const std::string_view& literal,
                              const std::string_view& line, State& state) {
    size_t start = state.index;
    size_t index = state.index;
    while (index < line.length() && index - start < literal.length() &&
           ::tolower(literal[index - start]) == ::tolower(line[index])) {
        index++;
    }

    state.index = index;

    if (index - start != literal.length()) {
        if (index - start != 0) {
            state.is_malformed = true;
        }

        return false;
    }

    return true;
}
}  // namespace simple_http
