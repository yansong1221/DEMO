#include "common/misc.h"

namespace common::misc {
std::string to_u8string(const char* local_str)
{
    try {
        auto s = std::filesystem::path(local_str).u8string();
        return std::string(s.begin(), s.end());
    }
    catch (...) {
    }
    return local_str;
}

std::string to_u8string(const std::string& s)
{
    return to_u8string(s.c_str());
}

std::string to_u8string(const std::filesystem::path& p)
{
    try {
        auto s = p.u8string();
        return std::string(s.begin(), s.end());
    }
    catch (...) {
    }
    return p.string();
}

std::string to_local8bit(const std::string& utf8_str)
{
    try {
        auto s = std::filesystem::u8path(utf8_str);
        return s.string();
    }
    catch (...) {
    }
    return utf8_str;
}

std::string wstring_to_u8string(const std::wstring& wstr)
{
    try {
        auto s = std::filesystem::path(wstr).u8string();
        return std::string(s.begin(), s.end());
    }
    catch (...) {
    }
    return std::string();
}

std::wstring u8string_to_wstring(const std::string& str)
{
    try {
        auto s = std::filesystem::u8path(str);
        return s.wstring();
    }
    catch (...) {
    }
    return std::wstring();
}

} // namespace common::misc