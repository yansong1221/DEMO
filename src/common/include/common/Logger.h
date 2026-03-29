#pragma once

#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/logservice/LogService.hpp>
#include <cppmicroservices/logservice/Logger.hpp>
#include <memory>
#include <string>

namespace common {

// 日志门面类 - 简化 cppmicroservices::logservice::Logger 的使用
class Logger
{
public:
    // 从 BundleContext 获取 Logger（推荐 bundles 使用）
    static std::shared_ptr<cppmicroservices::logservice::Logger>
    getLogger(cppmicroservices::BundleContext context, const std::string& name = {});

    // 从 Framework 获取 Logger（推荐 launcher 使用）
    static std::shared_ptr<cppmicroservices::logservice::Logger>
    getLogger(cppmicroservices::Framework& framework, const std::string& name = {});

    // 便捷日志方法 - 通过 BundleContext 直接输出日志
    static void info(cppmicroservices::BundleContext context, const std::string& message);
    static void warn(cppmicroservices::BundleContext context, const std::string& message);
    static void error(cppmicroservices::BundleContext context, const std::string& message);
    static void debug(cppmicroservices::BundleContext context, const std::string& message);
    static void trace(cppmicroservices::BundleContext context, const std::string& message);

    // 便捷日志方法 - 通过 Framework 直接输出日志（launcher 使用）
    static void info(cppmicroservices::Framework& framework, const std::string& message);
    static void warn(cppmicroservices::Framework& framework, const std::string& message);
    static void error(cppmicroservices::Framework& framework, const std::string& message);
    static void debug(cppmicroservices::Framework& framework, const std::string& message);
    static void trace(cppmicroservices::Framework& framework, const std::string& message);

private:
    static std::shared_ptr<cppmicroservices::logservice::LoggerFactory>
    getLoggerFactory(cppmicroservices::BundleContext context);
};

} // namespace common
