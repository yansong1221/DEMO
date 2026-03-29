#include "common/Logger.h"

#include <cppmicroservices/Bundle.h>

namespace common {

// 静态成员定义
static std::shared_ptr<cppmicroservices::logservice::Logger> s_logger;
static cppmicroservices::BundleContext s_context;
static std::mutex s_mutex;

namespace detail {

static std::shared_ptr<cppmicroservices::logservice::LoggerFactory>
getLoggerFactory(cppmicroservices::BundleContext context)
{
    if (!context)
        return nullptr;

    auto ref = context.GetServiceReference<cppmicroservices::logservice::LoggerFactory>();
    if (!ref) {
        return nullptr;
    }
    return context.GetService<cppmicroservices::logservice::LoggerFactory>(ref);
}
// ========== 从 BundleContext/Framework 获取 Logger ==========
static std::shared_ptr<cppmicroservices::logservice::Logger>
getLogger(cppmicroservices::BundleContext context,
          const std::string& name = cppmicroservices::logservice::LoggerFactory::ROOT_LOGGER_NAME)
{
    auto factory = getLoggerFactory(context);
    if (!factory) {
        return nullptr;
    }
    return factory->getLogger(context.GetBundle(), name);
}
} // namespace detail

void Logger::init(cppmicroservices::BundleContext context)
{
    std::lock_guard<std::mutex> lock(s_mutex);

    s_context = context;
    s_logger  = detail::getLogger(context);
}

void Logger::reset()
{
    std::lock_guard<std::mutex> lock(s_mutex);
    s_logger  = nullptr;
    s_context = nullptr;
}

void Logger::info(const std::string& message)
{
    if (auto logger = getLogger(); logger)
        logger->info(message);
}

void Logger::warn(const std::string& message)
{
    if (auto logger = getLogger(); logger)
        logger->warn(message);
}

void Logger::error(const std::string& message)
{
    if (auto logger = getLogger(); logger)
        logger->error(message);
}

void Logger::debug(const std::string& message)
{
    if (auto logger = getLogger(); logger)
        logger->debug(message);
}

std::shared_ptr<cppmicroservices::logservice::Logger> Logger::getLogger()
{
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_context)
        return nullptr;
    if (!s_logger) {
        s_logger = detail::getLogger(s_context);
    }
    return s_logger;
}

} // namespace common
