#pragma once
#include <format>
#include <string>

namespace cppmicroservices {
class BundleContext;
}

namespace common {

class logger
{
public:
    static void init(cppmicroservices::BundleContext const& context);
    static void reset();

    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);
    static void debug(const std::string& message);
    static void trace(const std::string& message);

    template<class... _Types>
    inline static void info(const std::format_string<_Types...> _Fmt, _Types&&... _Args)
    {
        logger::info(std::format(_Fmt, std::forward<_Types>(_Args)...));
    }
    template<class... _Types>
    inline static void warn(const std::format_string<_Types...> _Fmt, _Types&&... _Args)
    {
        logger::warn(std::format(_Fmt, std::forward<_Types>(_Args)...));
    }
    template<class... _Types>
    inline static void error(const std::format_string<_Types...> _Fmt, _Types&&... _Args)
    {
        logger::error(std::format(_Fmt, std::forward<_Types>(_Args)...));
    }
    template<class... _Types>
    inline static void debug(const std::format_string<_Types...> _Fmt, _Types&&... _Args)
    {
        logger::debug(std::format(_Fmt, std::forward<_Types>(_Args)...));
    }
    template<class... _Types>
    inline static void trace(const std::format_string<_Types...> _Fmt, _Types&&... _Args)
    {
        logger::trace(std::format(_Fmt, std::forward<_Types>(_Args)...));
    }
};

} // namespace common
