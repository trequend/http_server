// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

#include "http_headers.h"
#include "outgoing_message.h"

namespace simple_http {

void PrintHeaders(const HttpHeaders& headers);

std::string GetMimeType(const std::wstring& extension);

std::optional<std::filesystem::path> GetRequestFilePath(
    const std::string& request_path, const std::filesystem::path& base);

void ResponseWithFile(OutgoingMessage& response, const std::string& code,
                      const std::string& message,
                      const std::filesystem::path& file_path);

}  // namespace simple_http
