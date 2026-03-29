#include "common/Logger.h"

namespace common {

std::shared_ptr<cppmicroservices::logservice::LoggerFactory> Logger::getLoggerFactory(
    cppmicroservices::BundleContext context)
{
    auto ref = context.GetServiceReference<cppmicroservices::logservice::LoggerFactory>();
    if (!ref) {
        return nullptr;
    }
    return context.GetService<cppmicroservices::logservice::LoggerFactory>(ref);
}

std::shared_ptr<cppmicroservices::logservice::Logger> Logger::getLogger(
    cppmicroservices::BundleContext context, 
    const std::string& name)
{
    auto factory = getLoggerFactory(context);
    if (!factory) {
        return nullptr;
    }
    return factory->getLogger(name);
}

std::shared_ptr<cppmicroservices::logservice::Logger> Logger::getLogger(
    cppmicroservices::Framework& framework, 
    const std::string& name)
{
    return getLogger(framework.GetBundleContext(), name);
}

void Logger::info(cppmicroservices::BundleContext context, const std::string& message)
{
    auto logger = getLogger(context);
    if (logger) {
        logger->info(message);
    }
}

void Logger::warn(cppmicroservices::BundleContext context, const std::string& message)
{
    auto logger = getLogger(context);
    if (logger) {
        logger->warn(message);
    }
}

void Logger::error(cppmicroservices::BundleContext context, const std::string& message)
{
    auto logger = getLogger(context);
    if (logger) {
        logger->error(message);
    }
}

void Logger::debug(cppmicroservices::BundleContext context, const std::string& message)
{
    auto logger = getLogger(context);
    if (logger) {
        logger->debug(message);
    }
}

void Logger::trace(cppmicroservices::BundleContext context, const std::string& message)
{
    auto logger = getLogger(context);
    if (logger) {
        logger->trace(message);
    }
}

void Logger::info(cppmicroservices::Framework& framework, const std::string& message)
{
    info(framework.GetBundleContext(), message);
}

void Logger::warn(cppmicroservices::Framework& framework, const std::string& message)
{
    warn(framework.GetBundleContext(), message);
}

void Logger::error(cppmicroservices::Framework& framework, const std::string& message)
{
    error(framework.GetBundleContext(), message);
}

void Logger::debug(cppmicroservices::Framework& framework, const std::string& message)
{
    debug(framework.GetBundleContext(), message);
}

void Logger::trace(cppmicroservices::Framework& framework, const std::string& message)
{
    trace(framework.GetBundleContext(), message);
}

} // namespace demo
