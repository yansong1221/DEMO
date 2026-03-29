#pragma once

#include <cppmicroservices/logservice/LogService.hpp>
#include <cppmicroservices/logservice/Logger.hpp>
#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/ServiceReferenceBase.h>

#include <QObject>
#include <QString>
#include <memory>
#include <mutex>
#include <vector>
#include <functional>

class LogWidget;

// 日志级别映射：
// LogService::SeverityLevel -> LogWidget level
// LOG_ERROR -> 0, LOG_WARNING -> 1, LOG_INFO -> 2, LOG_DEBUG -> 3
// Logger::LogLevel -> LogWidget level  
// Error -> 0, Warn -> 1, Info -> 2, Debug -> 3, Trace -> 4, Audit -> 5

class LogServiceImpl;

// Logger 实现
class LoggerImpl : public cppmicroservices::logservice::Logger
{
public:
    LoggerImpl(std::string name, LogServiceImpl* service);
    LoggerImpl(std::string name, cppmicroservices::Bundle bundle, LogServiceImpl* service);
    ~LoggerImpl() override = default;

    // Logger interface
    void audit(std::string const& message) override;
    void audit(std::string const& format, std::string const& arg) override;
    void audit(std::string const& format, std::string const& arg1, std::string const& arg2) override;
    void audit(std::string const& message, const std::exception_ptr ex) override;
    void audit(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr) override;
    void audit(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr, const std::exception_ptr ex) override;

    void debug(std::string const& message) override;
    void debug(std::string const& format, std::string const& arg) override;
    void debug(std::string const& format, std::string const& arg1, std::string const& arg2) override;
    void debug(std::string const& message, const std::exception_ptr ex) override;
    void debug(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr) override;
    void debug(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr, const std::exception_ptr ex) override;

    void error(std::string const& message) override;
    void error(std::string const& format, std::string const& arg) override;
    void error(std::string const& format, std::string const& arg1, std::string const& arg2) override;
    void error(std::string const& message, const std::exception_ptr ex) override;
    void error(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr) override;
    void error(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr, const std::exception_ptr ex) override;

    void info(std::string const& message) override;
    void info(std::string const& format, std::string const& arg) override;
    void info(std::string const& format, std::string const& arg1, std::string const& arg2) override;
    void info(std::string const& message, const std::exception_ptr ex) override;
    void info(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr) override;
    void info(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr, const std::exception_ptr ex) override;

    void trace(std::string const& message) override;
    void trace(std::string const& format, std::string const& arg) override;
    void trace(std::string const& format, std::string const& arg1, std::string const& arg2) override;
    void trace(std::string const& message, const std::exception_ptr ex) override;
    void trace(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr) override;
    void trace(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr, const std::exception_ptr ex) override;

    void warn(std::string const& message) override;
    void warn(std::string const& format, std::string const& arg) override;
    void warn(std::string const& format, std::string const& arg1, std::string const& arg2) override;
    void warn(std::string const& message, const std::exception_ptr ex) override;
    void warn(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr) override;
    void warn(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr, const std::exception_ptr ex) override;

private:
    void log(int level, const std::string& message, cppmicroservices::ServiceReferenceBase const* sr = nullptr, const std::exception_ptr* ex = nullptr);
    void log(int level, const std::string& format, const std::string& arg);
    void log(int level, const std::string& format, const std::string& arg1, const std::string& arg2);
    QString formatException(const std::exception_ptr& ex);

    std::string m_name;
    cppmicroservices::Bundle m_bundle;
    LogServiceImpl* m_service;
};

// LogService 实现
class LogServiceImpl : public cppmicroservices::logservice::LogService
{
public:
    LogServiceImpl();
    ~LogServiceImpl() override = default;

    // 设置日志显示控件
    void setLogWidget(LogWidget* widget);
    
    // 获取日志控件（用于直接操作）
    LogWidget* getLogWidget() const;

    // LoggerFactory interface
    std::shared_ptr<cppmicroservices::logservice::Logger> getLogger(std::string const& name = ROOT_LOGGER_NAME) const override;
    std::shared_ptr<cppmicroservices::logservice::Logger> getLogger(const cppmicroservices::Bundle& bundle, std::string const& name = ROOT_LOGGER_NAME) const override;

    // LogService interface
    void Log(cppmicroservices::logservice::SeverityLevel level, std::string const& message) override;
    void Log(cppmicroservices::logservice::SeverityLevel level, std::string const& message, const std::exception_ptr ex) override;
    void Log(cppmicroservices::ServiceReferenceBase const& sr, cppmicroservices::logservice::SeverityLevel level, std::string const& message) override;
    void Log(cppmicroservices::ServiceReferenceBase const& sr, cppmicroservices::logservice::SeverityLevel level, std::string const& message, const std::exception_ptr ex) override;

    // 内部方法，供 LoggerImpl 调用
    void addLogEntry(int level, const QString& bundleName, const QString& message);

private:
    int severityToLevel(cppmicroservices::logservice::SeverityLevel level) const;
    QString getBundleName(const cppmicroservices::Bundle& bundle) const;

    mutable std::mutex m_mutex;
    LogWidget* m_logWidget = nullptr;
    std::vector<std::weak_ptr<LoggerImpl>> m_loggers;
};
