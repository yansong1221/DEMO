#pragma once
#include <format>
#include <string>

namespace cppmicroservices
{
    class BundleContext;
}

namespace common
{

    class Log
    {
      public:
        static void init(cppmicroservices::BundleContext const& context);
        static void reset();

        static void info(std::string const& message);
        static void warn(std::string const& message);
        static void error(std::string const& message);
        static void debug(std::string const& message);
        static void trace(std::string const& message);

        template <class... _Types>
        inline static void
        info(std::format_string<_Types...> const _Fmt, _Types&&... _Args)
        {
            Log::info(std::format(_Fmt, std::forward<_Types>(_Args)...));
        }
        template <class... _Types>
        inline static void
        warn(std::format_string<_Types...> const _Fmt, _Types&&... _Args)
        {
            Log::warn(std::format(_Fmt, std::forward<_Types>(_Args)...));
        }
        template <class... _Types>
        inline static void
        error(std::format_string<_Types...> const _Fmt, _Types&&... _Args)
        {
            Log::error(std::format(_Fmt, std::forward<_Types>(_Args)...));
        }
        template <class... _Types>
        inline static void
        debug(std::format_string<_Types...> const _Fmt, _Types&&... _Args)
        {
            Log::debug(std::format(_Fmt, std::forward<_Types>(_Args)...));
        }
        template <class... _Types>
        inline static void
        trace(std::format_string<_Types...> const _Fmt, _Types&&... _Args)
        {
            Log::trace(std::format(_Fmt, std::forward<_Types>(_Args)...));
        }
    };

} // namespace common
