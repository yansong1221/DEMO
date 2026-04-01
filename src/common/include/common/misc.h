#pragma once
#include <filesystem>
#include <string>

namespace common::misc
{

    std::string to_u8string(char const* local_str);
    std::string to_u8string(std::string const& local_str);
    std::string to_u8string(std::filesystem::path const& p);
    std::string to_local8bit(std::string const& utf8_str);
    std::string wstring_to_u8string(std::wstring const& wstr);
    std::wstring u8string_to_wstring(std::string const& str);

} // namespace common::misc