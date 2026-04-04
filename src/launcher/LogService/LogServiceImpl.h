#pragma once

#include <cppmicroservices/Bundle.h>
#include <cppmicroservices/ServiceReferenceBase.h>
#include <cppmicroservices/logservice/LogService.hpp>
#include <cppmicroservices/logservice/Logger.hpp>

#include "imgui_extend/imguial_term.h"
#include "service/IUIService.h"
#include <QObject>
#include <QPointer>
#include <QString>
#include <functional>
#include <memory>
#include <mutex>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/spdlog.h>
#include <vector>

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

// LogService 实现
class LogServiceImpl
    : public QObject
    , public spdlog::sinks::base_sink<spdlog::details::null_mutex>
    , public cppmicroservices::logservice::LogService
    , public service::IImGuiDrawService
    , public std::enable_shared_from_this<LogServiceImpl>
{
    Q_OBJECT
  public:
    LogServiceImpl();
    ~LogServiceImpl() override;

    void Init();

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

  protected:
    void sink_it_(spdlog::details::log_msg const& msg) override;
    void flush_() override;

  protected:
    void drawImGui() override;

  private:
    static spdlog::level::level_enum severityToLevel(cppmicroservices::logservice::SeverityLevel level);
    QString getBundleName(cppmicroservices::Bundle const& bundle) const;

    mutable std::mutex m_mutex;
    std::shared_ptr<spdlog::logger> m_logger;

    ImGuiAl::Log _terminal;
};
