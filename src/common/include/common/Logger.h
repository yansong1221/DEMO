#pragma once

#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/logservice/LogService.hpp>
#include <cppmicroservices/logservice/Logger.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace common {

// 日志门面类 - 简化 cppmicroservices::logservice::Logger 的使用
// 支持两种方式：
// 1. 全局模式：先调用 init() 设置 BundleContext，之后可直接使用 info/warn/error/debug/trace
// 2. 显式模式：每次调用时传入 BundleContext 或 Framework
class Logger
{
public:
    // ========== 全局初始化 ==========
    // 初始化全局 BundleContext（通常在 launcher 启动时调用一次）
    static void init(cppmicroservices::BundleContext context);
    // 重置全局 BundleContext
    static void reset();
    // 检查是否已初始化
    static bool isInitialized();

    // ========== 从 BundleContext/Framework 获取 Logger ==========
    static std::shared_ptr<cppmicroservices::logservice::Logger>
    getLogger(cppmicroservices::BundleContext context, const std::string& name = {});

    // 从全局 BundleContext 获取 Logger（需先调用 init）
    static std::shared_ptr<cppmicroservices::logservice::Logger>
    getLogger(const std::string& name = {});

    // ========== 便捷日志方法 - 显式传入 BundleContext ==========
    static void info(cppmicroservices::BundleContext context, const std::string& message);
    static void warn(cppmicroservices::BundleContext context, const std::string& message);
    static void error(cppmicroservices::BundleContext context, const std::string& message);
    static void debug(cppmicroservices::BundleContext context, const std::string& message);

    // ========== 便捷日志方法 - 使用全局 BundleContext（需先调用 init）==========
    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);
    static void debug(const std::string& message);

private:
    // 获取所有 LogService 服务引用
    static std::vector<cppmicroservices::ServiceReference<cppmicroservices::logservice::LogService>>
    getLogServiceRefs(cppmicroservices::BundleContext context);

    static std::shared_ptr<cppmicroservices::logservice::LoggerFactory>
    getLoggerFactory(cppmicroservices::BundleContext context);

    // 全局 BundleContext（使用 weak_ptr 避免影响框架生命周期）
    static cppmicroservices::BundleContext s_globalContext;
};

} // namespace common
