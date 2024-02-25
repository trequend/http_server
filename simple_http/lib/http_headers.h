// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace simple_http {

class HttpHeaders {
   public:
    void add(const std::string& name, const std::string& value);
    void add(std::string&& name, std::string&& value);

    std::optional<std::vector<std::string>> get(const std::string& name) const;

    std::map<std::string, std::vector<std::string>>::const_iterator find(
        const std::string& name) const {
        return headers_.find(name);
    };
    std::map<std::string, std::vector<std::string>>::const_iterator begin()
        const {
        return headers_.begin();
    };
    std::map<std::string, std::vector<std::string>>::const_iterator end()
        const {
        return headers_.end();
    };

   private:
    std::map<std::string, std::vector<std::string>> headers_;
};

};  // namespace simple_http
