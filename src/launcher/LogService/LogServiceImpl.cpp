#include "LogServiceImpl.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QMetaObject>

#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

// LoggerImpl implementation

LoggerImpl::LoggerImpl(std::string name, LogServiceImpl* service) : m_name(std::move(name)), m_service(service) {}

LoggerImpl::LoggerImpl(std::string name, cppmicroservices::Bundle bundle, LogServiceImpl* service)
    : m_name(std::move(name))
    , m_bundle(std::move(bundle))
    , m_service(service)
{
}

void
LoggerImpl::log(spdlog::level::level_enum level,
                std::string const& message,
                cppmicroservices::ServiceReferenceBase const* sr,
                std::exception_ptr const* ex)
{
    QString msg = QString::fromStdString(message);

    if (ex && *ex)
    {
        msg += " - Exception: " + formatException(*ex);
    }

    QString bundleName;
    if (sr && sr->GetBundle())
    {
        bundleName = QString::fromStdString(sr->GetBundle().GetSymbolicName());
    }
    else if (m_bundle)
    {
        bundleName = QString::fromStdString(m_bundle.GetSymbolicName());
    }
    else
    {
        bundleName = QString::fromStdString(m_name);
    }

    if (m_service)
    {
        m_service->addLogEntry(level, bundleName, msg);
    }
}

void
LoggerImpl::log(spdlog::level::level_enum level, std::string const& format, std::string const& arg)
{
    QString msg = QString::fromStdString(format).arg(QString::fromStdString(arg));
    QString bundleName = m_bundle ? QString::fromStdString(m_bundle.GetSymbolicName()) : QString::fromStdString(m_name);

    if (m_service)
    {
        m_service->addLogEntry(level, bundleName, msg);
    }
}

void
LoggerImpl::log(spdlog::level::level_enum level,
                std::string const& format,
                std::string const& arg1,
                std::string const& arg2)
{
    QString msg = QString::fromStdString(format).arg(QString::fromStdString(arg1), QString::fromStdString(arg2));
    QString bundleName = m_bundle ? QString::fromStdString(m_bundle.GetSymbolicName()) : QString::fromStdString(m_name);

    if (m_service)
    {
        m_service->addLogEntry(level, bundleName, msg);
    }
}

QString
LoggerImpl::formatException(std::exception_ptr const& ex)
{
    if (!ex)
    {
        return QString();
    }

    try
    {
        std::rethrow_exception(ex);
    }
    catch (std::exception const& e)
    {
        return QString::fromStdString(e.what());
    }
    catch (...)
    {
        return QString("Unknown exception");
    }
}

// Audit methods
void
LoggerImpl::audit(std::string const& message)
{
    log(spdlog::level::level_enum::critical, message);
}
void
LoggerImpl::audit(std::string const& format, std::string const& arg)
{
    log(spdlog::level::level_enum::critical, format, arg);
}
void
LoggerImpl::audit(std::string const& format, std::string const& arg1, std::string const& arg2)
{
    log(spdlog::level::level_enum::critical, format, arg1, arg2);
}
void
LoggerImpl::audit(std::string const& message, std::exception_ptr const ex)
{
    log(spdlog::level::level_enum::critical, message, nullptr, &ex);
}
void
LoggerImpl::audit(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr)
{
    log(spdlog::level::level_enum::critical, message, &sr);
}
void
LoggerImpl::audit(std::string const& message,
                  cppmicroservices::ServiceReferenceBase const& sr,
                  std::exception_ptr const ex)
{
    log(spdlog::level::level_enum::critical, message, &sr, &ex);
}

// Debug methods
void
LoggerImpl::debug(std::string const& message)
{
    log(spdlog::level::level_enum::debug, message);
}
void
LoggerImpl::debug(std::string const& format, std::string const& arg)
{
    log(spdlog::level::level_enum::debug, format, arg);
}
void
LoggerImpl::debug(std::string const& format, std::string const& arg1, std::string const& arg2)
{
    log(spdlog::level::level_enum::debug, format, arg1, arg2);
}
void
LoggerImpl::debug(std::string const& message, std::exception_ptr const ex)
{
    log(spdlog::level::level_enum::debug, message, nullptr, &ex);
}
void
LoggerImpl::debug(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr)
{
    log(spdlog::level::level_enum::debug, message, &sr);
}
void
LoggerImpl::debug(std::string const& message,
                  cppmicroservices::ServiceReferenceBase const& sr,
                  std::exception_ptr const ex)
{
    log(spdlog::level::level_enum::debug, message, &sr, &ex);
}

// Error methods
void
LoggerImpl::error(std::string const& message)
{
    log(spdlog::level::level_enum::err, message);
}
void
LoggerImpl::error(std::string const& format, std::string const& arg)
{
    log(spdlog::level::level_enum::err, format, arg);
}
void
LoggerImpl::error(std::string const& format, std::string const& arg1, std::string const& arg2)
{
    log(spdlog::level::level_enum::err, format, arg1, arg2);
}
void
LoggerImpl::error(std::string const& message, std::exception_ptr const ex)
{
    log(spdlog::level::level_enum::err, message, nullptr, &ex);
}
void
LoggerImpl::error(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr)
{
    log(spdlog::level::level_enum::err, message, &sr);
}
void
LoggerImpl::error(std::string const& message,
                  cppmicroservices::ServiceReferenceBase const& sr,
                  std::exception_ptr const ex)
{
    log(spdlog::level::level_enum::err, message, &sr, &ex);
}

// Info methods
void
LoggerImpl::info(std::string const& message)
{
    log(spdlog::level::level_enum::info, message);
}
void
LoggerImpl::info(std::string const& format, std::string const& arg)
{
    log(spdlog::level::level_enum::info, format, arg);
}
void
LoggerImpl::info(std::string const& format, std::string const& arg1, std::string const& arg2)
{
    log(spdlog::level::level_enum::info, format, arg1, arg2);
}
void
LoggerImpl::info(std::string const& message, std::exception_ptr const ex)
{
    log(spdlog::level::level_enum::info, message, nullptr, &ex);
}
void
LoggerImpl::info(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr)
{
    log(spdlog::level::level_enum::info, message, &sr);
}
void
LoggerImpl::info(std::string const& message,
                 cppmicroservices::ServiceReferenceBase const& sr,
                 std::exception_ptr const ex)
{
    log(spdlog::level::level_enum::info, message, &sr, &ex);
}

// Trace methods
void
LoggerImpl::trace(std::string const& message)
{
    log(spdlog::level::level_enum::trace, message);
}
void
LoggerImpl::trace(std::string const& format, std::string const& arg)
{
    log(spdlog::level::level_enum::trace, format, arg);
}
void
LoggerImpl::trace(std::string const& format, std::string const& arg1, std::string const& arg2)
{
    log(spdlog::level::level_enum::trace, format, arg1, arg2);
}
void
LoggerImpl::trace(std::string const& message, std::exception_ptr const ex)
{
    log(spdlog::level::level_enum::trace, message, nullptr, &ex);
}
void
LoggerImpl::trace(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr)
{
    log(spdlog::level::level_enum::trace, message, &sr);
}
void
LoggerImpl::trace(std::string const& message,
                  cppmicroservices::ServiceReferenceBase const& sr,
                  std::exception_ptr const ex)
{
    log(spdlog::level::level_enum::trace, message, &sr, &ex);
}

// Warn methods
void
LoggerImpl::warn(std::string const& message)
{
    log(spdlog::level::level_enum::warn, message);
}
void
LoggerImpl::warn(std::string const& format, std::string const& arg)
{
    log(spdlog::level::level_enum::warn, format, arg);
}
void
LoggerImpl::warn(std::string const& format, std::string const& arg1, std::string const& arg2)
{
    log(spdlog::level::level_enum::warn, format, arg1, arg2);
}
void
LoggerImpl::warn(std::string const& message, std::exception_ptr const ex)
{
    log(spdlog::level::level_enum::warn, message, nullptr, &ex);
}
void
LoggerImpl::warn(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr)
{
    log(spdlog::level::level_enum::warn, message, &sr);
}
void
LoggerImpl::warn(std::string const& message,
                 cppmicroservices::ServiceReferenceBase const& sr,
                 std::exception_ptr const ex)
{
    log(spdlog::level::level_enum::warn, message, &sr, &ex);
}

LogServiceImpl::LogServiceImpl() : _terminal(10000) {}

// LogServiceImpl implementation

LogServiceImpl::~LogServiceImpl() { spdlog::shutdown(); }

void
LogServiceImpl::Init()
{
    // 获取日志目录（应用程序目录下的 logs）
    QString logDir = QCoreApplication::applicationDirPath() + "/logs";
    QDir dir(logDir);
    if (!dir.exists())
    {
        dir.mkpath(".");
    }
    try
    {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::trace);

        auto ui_sink = shared_from_this();
        ui_sink->set_level(spdlog::level::trace);

        auto file_name_pattern = fmt::format("{}\\%Y-%m-%d.log", logDir.toStdString());

        auto file_sink = std::make_shared<spdlog::sinks::daily_file_format_sink_mt>(file_name_pattern, 0, 0, false, 30);
        file_sink->set_level(spdlog::level::trace);

        spdlog::sinks_init_list sink_list = { file_sink, ui_sink, console_sink };
        m_logger = std::make_shared<spdlog::logger>("logger", sink_list);
        m_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
        m_logger->set_level(spdlog::level::info);
        m_logger->flush_on(spdlog::level::info);

        // m_fileLogger->set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] [%n] %v");
        // m_fileLogger->set_level(spdlog::level::trace);
    }
    catch (spdlog::spdlog_ex const& ex)
    {
        spdlog::error("File logger init failed: {}", ex.what());
    }
}

std::shared_ptr<cppmicroservices::logservice::Logger>
LogServiceImpl::getLogger(std::string const& name) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return std::make_shared<LoggerImpl>(name, const_cast<LogServiceImpl*>(this));
}

std::shared_ptr<cppmicroservices::logservice::Logger>
LogServiceImpl::getLogger(cppmicroservices::Bundle const& bundle, std::string const& name) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return std::make_shared<LoggerImpl>(name, bundle, const_cast<LogServiceImpl*>(this));
}

void
LogServiceImpl::Log(cppmicroservices::logservice::SeverityLevel level, std::string const& message)
{
    QString bundleName = "system";
    QString msg = QString::fromStdString(message);

    addLogEntry(level, bundleName, msg);
}

void
LogServiceImpl::Log(cppmicroservices::logservice::SeverityLevel level,
                    std::string const& message,
                    std::exception_ptr const ex)
{
    QString bundleName = "system";
    QString msg = QString::fromStdString(message);

    if (ex)
    {
        try
        {
            std::rethrow_exception(ex);
        }
        catch (std::exception const& e)
        {
            msg += " - Exception: " + QString::fromStdString(e.what());
        }
        catch (...)
        {
            msg += " - Unknown exception";
        }
    }

    addLogEntry(level, bundleName, msg);
}

void
LogServiceImpl::Log(cppmicroservices::ServiceReferenceBase const& sr,
                    cppmicroservices::logservice::SeverityLevel level,
                    std::string const& message)
{
    QString bundleName = sr.GetBundle() ? QString::fromStdString(sr.GetBundle().GetSymbolicName()) : "unknown";
    QString msg = QString::fromStdString(message);

    addLogEntry(level, bundleName, msg);
}

void
LogServiceImpl::Log(cppmicroservices::ServiceReferenceBase const& sr,
                    cppmicroservices::logservice::SeverityLevel level,
                    std::string const& message,
                    std::exception_ptr const ex)
{
    QString bundleName = sr.GetBundle() ? QString::fromStdString(sr.GetBundle().GetSymbolicName()) : "unknown";
    QString msg = QString::fromStdString(message);

    if (ex)
    {
        try
        {
            std::rethrow_exception(ex);
        }
        catch (std::exception const& e)
        {
            msg += " - Exception: " + QString::fromStdString(e.what());
        }
        catch (...)
        {
            msg += " - Unknown exception";
        }
    }

    addLogEntry(level, bundleName, msg);
}

void
LogServiceImpl::addLogEntry(spdlog::level::level_enum level, QString const& bundleName, QString const& message)
{
    // std::lock_guard<std::mutex> lock(m_mutex);

    //// UI 更新
    // if (m_logWidget)
    //{
    //     QMetaObject::invokeMethod(
    //         m_logWidget,
    //         [this, level, bundleName, message]() { m_logWidget->addLog(level, bundleName, message); },
    //         Qt::QueuedConnection);
    // }

    // spdlog 输出（自动异步，线程安全）

    m_logger->log(level, "[{}] {}", bundleName.toStdString(), message.toStdString());
}

void
LogServiceImpl::addLogEntry(cppmicroservices::logservice::SeverityLevel level,
                            QString const& bundleName,
                            QString const& message)
{
    addLogEntry(severityToLevel(level), bundleName, message);
}

spdlog::level::level_enum
LogServiceImpl::severityToLevel(cppmicroservices::logservice::SeverityLevel level)
{
    switch (level)
    {
        case cppmicroservices::logservice::SeverityLevel::LOG_ERROR:
            return spdlog::level::level_enum::err;
        case cppmicroservices::logservice::SeverityLevel::LOG_WARNING:
            return spdlog::level::level_enum::warn;
        case cppmicroservices::logservice::SeverityLevel::LOG_INFO:
            return spdlog::level::level_enum::info;
        case cppmicroservices::logservice::SeverityLevel::LOG_DEBUG:
            return spdlog::level::level_enum::debug;
        default:
            return spdlog::level::level_enum::info;
    }
}

QString
LogServiceImpl::getBundleName(cppmicroservices::Bundle const& bundle) const
{
    if (!bundle)
    {
        return "system";
    }
    return QString::fromStdString(bundle.GetSymbolicName());
}
void
LogServiceImpl::sink_it_(spdlog::details::log_msg const& msg)
{
    spdlog::memory_buf_t formatted;
    formatter_->format(msg, formatted);
    auto message = QString::fromUtf8(formatted.data(), formatted.size());

    QMetaObject::invokeMethod(
        this,
        [this, level = msg.level, message = std::move(message)]()
        {
            switch (level)
            {
                case spdlog::level::trace:
                    _terminal.intput(ImGuiAl::Log::Level::Trace, message.toStdString());
                    break;
                case spdlog::level::debug:
                    _terminal.intput(ImGuiAl::Log::Level::Debug, message.toStdString());
                    break;
                case spdlog::level::info:
                    _terminal.intput(ImGuiAl::Log::Level::Info, message.toStdString());
                    break;
                case spdlog::level::warn:
                    _terminal.intput(ImGuiAl::Log::Level::Warning, message.toStdString());
                    break;
                case spdlog::level::err:
                    _terminal.intput(ImGuiAl::Log::Level::Error, message.toStdString());
                    break;
                case spdlog::level::critical:
                    _terminal.intput(ImGuiAl::Log::Level::Critical, message.toStdString());
                    break;
                default:
                    break;
            }
        },
        Qt::QueuedConnection);
}

void
LogServiceImpl::flush_()
{
}

void
LogServiceImpl::drawImGui()
{
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(tr("UILogOutput").toUtf8()))
    {
        _terminal.draw();
    }
    ImGui::End();
}
