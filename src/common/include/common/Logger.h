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

    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);
    static void debug(const std::string& message);

    static std::shared_ptr<cppmicroservices::logservice::Logger> getLogger();
};

} // namespace common
