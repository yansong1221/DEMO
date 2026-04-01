#pragma once
#include <filesystem>
#include <string>

namespace common::misc {

std::string to_u8string(const char* local_str);
std::string to_u8string(const std::string& local_str);
std::string to_u8string(const std::filesystem::path& p);
std::string to_local8bit(const std::string& utf8_str);
std::string wstring_to_u8string(const std::wstring& wstr);
std::wstring u8string_to_wstring(const std::string& str);

} // namespace common::misc