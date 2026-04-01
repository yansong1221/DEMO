#include "common/misc.h"

namespace common::misc
{
    std::string
    to_u8string(char const* local_str)
    {
        try
        {
            auto s = std::filesystem::path(local_str).u8string();
            return std::string(s.begin(), s.end());
        }
        catch (...)
        {
        }
        return local_str;
    }

    std::string
    to_u8string(std::string const& s)
    {
        return to_u8string(s.c_str());
    }

    std::string
    to_u8string(std::filesystem::path const& p)
    {
        try
        {
            auto s = p.u8string();
            return std::string(s.begin(), s.end());
        }
        catch (...)
        {
        }
        return p.string();
    }

    std::string
    to_local8bit(std::string const& utf8_str)
    {
        try
        {
            auto s = std::filesystem::u8path(utf8_str);
            return s.string();
        }
        catch (...)
        {
        }
        return utf8_str;
    }

    std::string
    wstring_to_u8string(std::wstring const& wstr)
    {
        try
        {
            auto s = std::filesystem::path(wstr).u8string();
            return std::string(s.begin(), s.end());
        }
        catch (...)
        {
        }
        return std::string();
    }

    std::wstring
    u8string_to_wstring(std::string const& str)
    {
        try
        {
            auto s = std::filesystem::u8path(str);
            return s.wstring();
        }
        catch (...)
        {
        }
        return std::wstring();
    }

} // namespace common::misc