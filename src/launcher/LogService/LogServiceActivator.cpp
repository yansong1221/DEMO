#include "LogServiceActivator.h"
#include "LogServiceImpl.h"

// 静态成员初始化
LogServiceActivator* LogServiceActivator::s_instance = nullptr;

LogServiceActivator::LogServiceActivator() { s_instance = this; }

LogServiceActivator::~LogServiceActivator() { s_instance = nullptr; }

void
LogServiceActivator::Start(cppmicroservices::BundleContext context)
{
    // 创建 LogService 实现
    m_logServiceImpl = std::make_shared<LogServiceImpl>();

    // 设置服务属性
    cppmicroservices::ServiceProperties props;
    props["service.description"] = std::string("Qt-based LogService Implementation");
    props["service.vendor"] = std::string("CppMicroServices Demo");
    props["log.service.type"] = std::string("qt-widget");

    // 注册 LogService 服务
    m_serviceRegistration = context.RegisterService<cppmicroservices::logservice::LogService>(m_logServiceImpl, props);

    // 同时注册 LoggerFactory 服务（LogService 继承自 LoggerFactory）
    m_factoryRegistration
        = context.RegisterService<cppmicroservices::logservice::LoggerFactory>(m_logServiceImpl, props);

    // 记录启动日志
    if (m_logServiceImpl)
    {
        m_logServiceImpl->Log(cppmicroservices::logservice::SeverityLevel::LOG_INFO, "LogService started successfully");
    }
}

void
LogServiceActivator::Stop(cppmicroservices::BundleContext context)
{
    (void)context;

    // 记录停止日志
    if (m_logServiceImpl)
    {
        m_logServiceImpl->Log(cppmicroservices::logservice::SeverityLevel::LOG_INFO, "LogService stopping...");
    }

    // 注销服务
    m_serviceRegistration.Unregister();
    m_factoryRegistration.Unregister();

    // 释放实现
    m_logServiceImpl.reset();
}

LogServiceImpl*
LogServiceActivator::GetLogServiceImpl()
{
    if (s_instance && s_instance->m_logServiceImpl)
    {
        return s_instance->m_logServiceImpl.get();
    }
    return nullptr;
}
