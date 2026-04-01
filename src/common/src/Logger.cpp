#include "common/logger.h"
#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/logservice/LogService.hpp>
#include <cppmicroservices/logservice/Logger.hpp>
#include <memory>
#include <mutex>

namespace common
{

    static std::shared_ptr<cppmicroservices::logservice::Logger> s_logger;
    static cppmicroservices::BundleContext s_context;
    static std::mutex s_mutex;

    namespace detail
    {

        static std::shared_ptr<cppmicroservices::logservice::LoggerFactory>
        get_logger_factory(cppmicroservices::BundleContext context)
        {
            if (!context)
            {
                return nullptr;
            }

            auto ref = context.GetServiceReference<cppmicroservices::logservice::LoggerFactory>();
            if (!ref)
            {
                return nullptr;
            }
            return context.GetService<cppmicroservices::logservice::LoggerFactory>(ref);
        }

        static std::shared_ptr<cppmicroservices::logservice::Logger>
        get_logger(cppmicroservices::BundleContext context,
                   std::string const& name = cppmicroservices::logservice::LoggerFactory::ROOT_LOGGER_NAME)
        {
            auto factory = get_logger_factory(context);
            if (!factory)
            {
                return nullptr;
            }
            return factory->getLogger(context.GetBundle(), name);
        }
        static std::shared_ptr<cppmicroservices::logservice::Logger>
        get_logger()
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            if (!s_context)
            {
                return nullptr;
            }
            if (!s_logger)
            {
                s_logger = detail::get_logger(s_context);
            }
            return s_logger;
        }
    } // namespace detail

    void
    Log::init(cppmicroservices::BundleContext const& context)
    {
        std::lock_guard<std::mutex> lock(s_mutex);

        s_context = context;
        s_logger = detail::get_logger(context);
    }

    void
    Log::reset()
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_logger = nullptr;
        s_context = nullptr;
    }

    void
    Log::info(std::string const& message)
    {
        if (auto logger = detail::get_logger(); logger)
        {
            logger->info(message);
        }
    }

    void
    Log::warn(std::string const& message)
    {
        if (auto logger = detail::get_logger(); logger)
        {
            logger->warn(message);
        }
    }

    void
    Log::error(std::string const& message)
    {
        if (auto logger = detail::get_logger(); logger)
        {
            logger->error(message);
        }
    }

    void
    Log::debug(std::string const& message)
    {
        if (auto logger = detail::get_logger(); logger)
        {
            logger->debug(message);
        }
    }

    void
    Log::trace(std::string const& message)
    {
        if (auto logger = detail::get_logger(); logger)
        {
            logger->trace(message);
        }
    }

} // namespace common
