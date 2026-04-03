#pragma once

#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/ServiceReferenceBase.h>
#include <cppmicroservices/logservice/LogService.hpp>
#include <cppmicroservices/logservice/Logger.hpp>

#include <QObject>
#include <QPointer>
#include <QString>
#include <functional>
#include <memory>
#include <mutex>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/spdlog.h>
#include <vector>

class LogWidget;
class QTextEdit;

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
    void audit(std::string const& message, std::exception_ptr const ex) override;
    void audit(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr) override;
    void audit(std::string const& message,
               cppmicroservices::ServiceReferenceBase const& sr,
               std::exception_ptr const ex) override;

    void debug(std::string const& message) override;
    void debug(std::string const& format, std::string const& arg) override;
    void debug(std::string const& format, std::string const& arg1, std::string const& arg2) override;
    void debug(std::string const& message, std::exception_ptr const ex) override;
    void debug(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr) override;
    void debug(std::string const& message,
               cppmicroservices::ServiceReferenceBase const& sr,
               std::exception_ptr const ex) override;

    void error(std::string const& message) override;
    void error(std::string const& format, std::string const& arg) override;
    void error(std::string const& format, std::string const& arg1, std::string const& arg2) override;
    void error(std::string const& message, std::exception_ptr const ex) override;
    void error(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr) override;
    void error(std::string const& message,
               cppmicroservices::ServiceReferenceBase const& sr,
               std::exception_ptr const ex) override;

    void info(std::string const& message) override;
    void info(std::string const& format, std::string const& arg) override;
    void info(std::string const& format, std::string const& arg1, std::string const& arg2) override;
    void info(std::string const& message, std::exception_ptr const ex) override;
    void info(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr) override;
    void info(std::string const& message,
              cppmicroservices::ServiceReferenceBase const& sr,
              std::exception_ptr const ex) override;

    void trace(std::string const& message) override;
    void trace(std::string const& format, std::string const& arg) override;
    void trace(std::string const& format, std::string const& arg1, std::string const& arg2) override;
    void trace(std::string const& message, std::exception_ptr const ex) override;
    void trace(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr) override;
    void trace(std::string const& message,
               cppmicroservices::ServiceReferenceBase const& sr,
               std::exception_ptr const ex) override;

    void warn(std::string const& message) override;
    void warn(std::string const& format, std::string const& arg) override;
    void warn(std::string const& format, std::string const& arg1, std::string const& arg2) override;
    void warn(std::string const& message, std::exception_ptr const ex) override;
    void warn(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr) override;
    void warn(std::string const& message,
              cppmicroservices::ServiceReferenceBase const& sr,
              std::exception_ptr const ex) override;

  private:
    void log(spdlog::level::level_enum level,
             std::string const& message,
             cppmicroservices::ServiceReferenceBase const* sr = nullptr,
             std::exception_ptr const* ex = nullptr);
    void log(spdlog::level::level_enum level, std::string const& format, std::string const& arg);
    void log(spdlog::level::level_enum level,
             std::string const& format,
             std::string const& arg1,
             std::string const& arg2);
    QString formatException(std::exception_ptr const& ex);

    std::string m_name;
    cppmicroservices::Bundle m_bundle;
    LogServiceImpl* m_service;
};

class QtLogSink : public spdlog::sinks::base_sink<spdlog::details::null_mutex>

{
  public:
    QtLogSink(LogWidget* widget);

  protected:
    void sink_it_(spdlog::details::log_msg const& msg) override;
    void flush_() override;

  private:
    QPointer<LogWidget> widget_;
};

// LogService 实现
class LogServiceImpl : public cppmicroservices::logservice::LogService
{
  public:
    LogServiceImpl(LogWidget* widget);
    ~LogServiceImpl() override;

    // 获取日志控件（用于直接操作）
    LogWidget* getLogWidget() const;

    // LoggerFactory interface
    std::shared_ptr<cppmicroservices::logservice::Logger> getLogger(std::string const& name
                                                                    = ROOT_LOGGER_NAME) const override;
    std::shared_ptr<cppmicroservices::logservice::Logger> getLogger(cppmicroservices::Bundle const& bundle,
                                                                    std::string const& name
                                                                    = ROOT_LOGGER_NAME) const override;

    // LogService interface
    void Log(cppmicroservices::logservice::SeverityLevel level, std::string const& message) override;
    void Log(cppmicroservices::logservice::SeverityLevel level,
             std::string const& message,
             std::exception_ptr const ex) override;
    void Log(cppmicroservices::ServiceReferenceBase const& sr,
             cppmicroservices::logservice::SeverityLevel level,
             std::string const& message) override;
    void Log(cppmicroservices::ServiceReferenceBase const& sr,
             cppmicroservices::logservice::SeverityLevel level,
             std::string const& message,
             std::exception_ptr const ex) override;

    // 内部方法，供 LoggerImpl 调用
    void addLogEntry(spdlog::level::level_enum level, QString const& bundleName, QString const& message);
    void addLogEntry(cppmicroservices::logservice::SeverityLevel level,
                     QString const& bundleName,
                     QString const& message);

  private:
    static spdlog::level::level_enum severityToLevel(cppmicroservices::logservice::SeverityLevel level);
    QString getBundleName(cppmicroservices::Bundle const& bundle) const;

    mutable std::mutex m_mutex;
    LogWidget* m_logWidget;

    // spdlog 相关
    std::shared_ptr<spdlog::logger> m_logger; // 文件日志
};
