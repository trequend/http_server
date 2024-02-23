// Copyright 2024 Balakin Dmitry. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include "http_headers.h"

#include <algorithm>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

void simple_http::HttpHeaders::add(const std::string& name,
                                   const std::string& value) {
    std::string normilized_name = name;
    std::transform(normilized_name.begin(), normilized_name.end(),
                   normilized_name.begin(), ::tolower);

    auto it = headers_.find(normilized_name);
    if (it != headers_.end()) {
        it->second.push_back(value);
        return;
    }

    headers_.insert({normilized_name, {value}});
}

void simple_http::HttpHeaders::add(std::string&& name, std::string&& value) {
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);

    auto it = headers_.find(name);
    if (it != headers_.end()) {
        it->second.push_back(std::move(value));
        return;
    }

    headers_.insert({std::move(name), {std::move(value)}});
}

std::optional<std::vector<std::string>> simple_http::HttpHeaders::get(
    const std::string& name) const {
    auto it = headers_.find(name);
    if (it != headers_.end()) {
        return it->second;
    }

    return std::nullopt;
}
