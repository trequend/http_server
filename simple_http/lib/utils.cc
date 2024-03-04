// Copyright 2024 Dmitrii Balakin. All rights reserved.
// Use of this source code is governed by a MIT License that can be
// found in the LICENSE file.

#include "utils.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "http_headers.h"
#include "outgoing_message.h"

namespace simple_http {

void PrintHeaders(const HttpHeaders& headers) {
    for (auto headers_it = headers.begin(); headers_it != headers.end();
         headers_it++) {
        std::cout << headers_it->first << ": [" << std::endl;
        auto header_values = headers_it->second;
        for (auto header_it = header_values.begin();
             header_it != header_values.end(); header_it++) {
            std::cout << "  \"" << *header_it << "\"" << std::endl;
        }
        std::cout << "]" << std::endl;
    }
}

std::string GetMimeType(const std::wstring& extension) {
    static std::map<std::wstring, std::string> mime_types = {
        {L".txt", "text/plain"},
        {L".html", "text/html"},
        {L".css", "text/css"},
        {L".js", "application/javascript"},
        {L".json", "application/json"},
        {L".xml", "application/xml"},
        {L".gif", "image/gif"},
        {L".jpg", "image/jpeg"},
        {L".jpeg", "image/jpeg"},
        {L".png", "image/png"},
        {L".svg", "image/svg+xml"},
        {L".bmp", "image/bmp"},
        {L".ico", "image/x-icon"},
        {L".webp", "image/webp"},
        {L".mp3", "audio/mpeg"},
        {L".wav", "audio/wav"},
        {L".ogg", "audio/ogg"},
        {L".mp4", "video/mp4"},
        {L".webm", "video/webm"},
        {L".avi", "video/x-msvideo"},
        {L".mkv", "video/x-matroska"},
        {L".zip", "application/zip"},
        {L".rar", "application/x-rar-compressed"},
        {L".tar", "application/x-tar"},
        {L".gz", "application/gzip"},
        {L".bz2", "application/x-bzip2"},
        {L".7z", "application/x-7z-compressed"},
        {L".pdf", "application/pdf"},
        {L".doc", "application/msword"},
        {L".docx",
         "application/"
         "vnd.openxmlformats-officedocument.wordprocessingml.document"},
    };

    auto it = mime_types.find(extension);
    if (it != mime_types.end()) {
        return it->second;
    } else {
        return "application/octet-stream";
    }
}

std::optional<std::filesystem::path> GetRequestFilePath(
    const std::string& request_path, const std::filesystem::path& base) {
    std::string_view relative_path(request_path.c_str() + 1,
                                   request_path.length() - 1);
    std::filesystem::path file_path =
        std::filesystem::weakly_canonical(base / relative_path);
    if (!file_path.native().starts_with(base.native()) ||
        !std::filesystem::exists(file_path)) {
        return std::nullopt;
    }

    if (std::filesystem::is_directory(file_path)) {
        file_path /= "index.html";
    } else if (file_path.filename().generic_wstring().starts_with(L"_")) {
        return std::nullopt;
    }

    if (!std::filesystem::exists(file_path) ||
        !std::filesystem::is_regular_file(file_path)) {
        return std::nullopt;
    }

    return file_path;
}

void ResponseWithFile(OutgoingMessage& response, const std::string& code,
                      const std::string& message,
                      const std::filesystem::path& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        std::cout << "File opening error" << std::endl;
        return;
    }

    file.seekg(0, std::ios::end);
    std::streampos end_postion = file.tellg();
    size_t file_size = static_cast<size_t>(end_postion);
    file.seekg(0, std::ios::beg);

    simple_http::HttpHeaders& headers = response.getHeaders();
    headers.add("Content-Length", std::to_string(file_size));
    headers.add("Content-Type",
                GetMimeType(file_path.extension().generic_wstring()) +
                    "; charset=UTF-8");
    headers.add("X-Powered-By", "simple_http");

    response.writeHead(code, message);

    char read_buffer[4096];
    while (file.good()) {
        file.read(read_buffer, sizeof(read_buffer));
        size_t bytes_readed = file.gcount();
        if (bytes_readed > 0) {
            simple_http::OutgoingMessage::WriteError write_error;
            write_error = response.write(read_buffer, bytes_readed);
            if (write_error != simple_http::OutgoingMessage::WriteError::kOk) {
                std::cout << "Send error: " << static_cast<int>(write_error)
                          << std::endl;
                return;
            }
        }
    }

    if (file.bad() || !file.eof()) {
        std::cout << "File reading error" << std::endl;
        return;
    }

    file.close();
    response.end();
}

}  // namespace simple_http
