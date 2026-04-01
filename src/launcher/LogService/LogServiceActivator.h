#pragma once

#include <cppmicroservices/BundleActivator.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/ServiceProperties.h>
#include <cppmicroservices/ServiceRegistration.h>
#include <cppmicroservices/logservice/LogService.hpp>
#include <cppmicroservices/logservice/LoggerFactory.hpp>

#include <memory>

class LogServiceImpl;

/**
 * @brief LogService 的 BundleActivator
 *
 * 在 Bundle 启动时创建并注册 LogService 服务
 * 在 Bundle 停止时注销服务
 */
class LogServiceActivator : public cppmicroservices::BundleActivator
{
  public:
    LogServiceActivator();
    ~LogServiceActivator() override;

    /**
     * @brief Bundle 启动时调用
     * @param context Bundle 上下文
     */
    void Start(cppmicroservices::BundleContext context) override;

    /**
     * @brief Bundle 停止时调用
     * @param context Bundle 上下文
     */
    void Stop(cppmicroservices::BundleContext context) override;

    /**
     * @brief 获取 LogServiceImpl 实例（供宿主应用使用）
     * @return LogServiceImpl 指针
     */
    static LogServiceImpl* GetLogServiceImpl();

  private:
    static LogServiceActivator* s_instance;

    std::shared_ptr<LogServiceImpl> m_logServiceImpl;
    cppmicroservices::ServiceRegistration<cppmicroservices::logservice::LogService> m_serviceRegistration;
    cppmicroservices::ServiceRegistration<cppmicroservices::logservice::LoggerFactory> m_factoryRegistration;
};

// 导出 Bundle 激活函数
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(LogServiceActivator)
