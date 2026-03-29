#include "common/Logger.h"

namespace common {

// 静态成员定义
cppmicroservices::BundleContext Logger::s_globalContext;

// ========== 全局初始化 ==========

void Logger::init(cppmicroservices::BundleContext context)
{
    s_globalContext = context;
}

void Logger::reset()
{
    s_globalContext = nullptr;
}

bool Logger::isInitialized()
{
    return (bool)s_globalContext;
}

// ========== LogService 获取（所有实现）==========

std::vector<cppmicroservices::ServiceReference<cppmicroservices::logservice::LogService>>
Logger::getLogServiceRefs(cppmicroservices::BundleContext context)
{
    if (!context)
        return {};

    return context.GetServiceReferences<cppmicroservices::logservice::LogService>();
}

// ========== LoggerFactory 获取 ==========

std::shared_ptr<cppmicroservices::logservice::LoggerFactory>
Logger::getLoggerFactory(cppmicroservices::BundleContext context)
{
    if (!context)
        return nullptr;

    auto ref = context.GetServiceReference<cppmicroservices::logservice::LoggerFactory>();
    if (!ref) {
        return nullptr;
    }
    return context.GetService<cppmicroservices::logservice::LoggerFactory>(ref);
}

// ========== getLogger 重载 ==========

std::shared_ptr<cppmicroservices::logservice::Logger>
Logger::getLogger(cppmicroservices::BundleContext context, const std::string& name)
{
    auto factory = getLoggerFactory(context);
    if (!factory) {
        return nullptr;
    }
    return factory->getLogger(name);
}

std::shared_ptr<cppmicroservices::logservice::Logger> Logger::getLogger(const std::string& name)
{
    return getLogger(s_globalContext, name);
}

// ========== 显式 BundleContext 日志方法 ==========
// 输出到所有 LogService 实现

void Logger::info(cppmicroservices::BundleContext context, const std::string& message)
{
    auto refs = getLogServiceRefs(context);
    for (auto& ref : refs) {
        auto service = context.GetService<cppmicroservices::logservice::LogService>(ref);
        if (service) {
            service->Log(cppmicroservices::logservice::SeverityLevel::LOG_INFO, message);
        }
    }
}

void Logger::warn(cppmicroservices::BundleContext context, const std::string& message)
{
    auto refs = getLogServiceRefs(context);
    for (auto& ref : refs) {
        auto service = context.GetService<cppmicroservices::logservice::LogService>(ref);
        if (service) {
            service->Log(cppmicroservices::logservice::SeverityLevel::LOG_WARNING, message);
        }
    }
}

void Logger::error(cppmicroservices::BundleContext context, const std::string& message)
{
    auto refs = getLogServiceRefs(context);
    for (auto& ref : refs) {
        auto service = context.GetService<cppmicroservices::logservice::LogService>(ref);
        if (service) {
            service->Log(cppmicroservices::logservice::SeverityLevel::LOG_ERROR, message);
        }
    }
}

void Logger::debug(cppmicroservices::BundleContext context, const std::string& message)
{
    auto refs = getLogServiceRefs(context);
    for (auto& ref : refs) {
        auto service = context.GetService<cppmicroservices::logservice::LogService>(ref);
        if (service) {
            service->Log(cppmicroservices::logservice::SeverityLevel::LOG_DEBUG, message);
        }
    }
}

// ========== 全局日志方法（无需传入 context）==========

void Logger::info(const std::string& message)
{
    info(s_globalContext, message);
}

void Logger::warn(const std::string& message)
{
    warn(s_globalContext, message);
}

void Logger::error(const std::string& message)
{
    error(s_globalContext, message);
}

void Logger::debug(const std::string& message)
{
    debug(s_globalContext, message);
}

} // namespace common
