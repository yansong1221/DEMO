#include "LogServiceImpl.h"
#include "LogWidget.h"

#include <QMetaObject>
#include <QDateTime>
#include <QDir>
#include <QCoreApplication>

// LoggerImpl implementation

LoggerImpl::LoggerImpl(std::string name, LogServiceImpl* service)
    : m_name(std::move(name))
    , m_service(service)
{
}

LoggerImpl::LoggerImpl(std::string name, cppmicroservices::Bundle bundle, LogServiceImpl* service)
    : m_name(std::move(name))
    , m_bundle(std::move(bundle))
    , m_service(service)
{
}

void LoggerImpl::log(int level,
                     const std::string& message,
                     cppmicroservices::ServiceReferenceBase const* sr,
                     const std::exception_ptr* ex)
{
    QString msg = QString::fromStdString(message);

    if (ex && *ex) {
        msg += " - Exception: " + formatException(*ex);
    }

    QString bundleName;
    if (sr && sr->GetBundle()) {
        bundleName = QString::fromStdString(sr->GetBundle().GetSymbolicName());
    }
    else if (m_bundle) {
        bundleName = QString::fromStdString(m_bundle.GetSymbolicName());
    }
    else {
        bundleName = QString::fromStdString(m_name);
    }

    if (m_service) {
        m_service->addLogEntry(level, bundleName, msg);
    }
}

void LoggerImpl::log(int level, const std::string& format, const std::string& arg)
{
    QString msg        = QString::fromStdString(format).arg(QString::fromStdString(arg));
    QString bundleName = m_bundle ? QString::fromStdString(m_bundle.GetSymbolicName())
                                  : QString::fromStdString(m_name);

    if (m_service) {
        m_service->addLogEntry(level, bundleName, msg);
    }
}

void LoggerImpl::log(int level,
                     const std::string& format,
                     const std::string& arg1,
                     const std::string& arg2)
{
    QString msg        = QString::fromStdString(format).arg(QString::fromStdString(arg1),
                                                     QString::fromStdString(arg2));
    QString bundleName = m_bundle ? QString::fromStdString(m_bundle.GetSymbolicName())
                                  : QString::fromStdString(m_name);

    if (m_service) {
        m_service->addLogEntry(level, bundleName, msg);
    }
}

QString LoggerImpl::formatException(const std::exception_ptr& ex)
{
    if (!ex)
        return QString();

    try {
        std::rethrow_exception(ex);
    }
    catch (const std::exception& e) {
        return QString::fromStdString(e.what());
    }
    catch (...) {
        return QString("Unknown exception");
    }
}

// Audit methods
void LoggerImpl::audit(std::string const& message)
{
    log(5, message);
}
void LoggerImpl::audit(std::string const& format, std::string const& arg)
{
    log(5, format, arg);
}
void LoggerImpl::audit(std::string const& format, std::string const& arg1, std::string const& arg2)
{
    log(5, format, arg1, arg2);
}
void LoggerImpl::audit(std::string const& message, const std::exception_ptr ex)
{
    log(5, message, nullptr, &ex);
}
void LoggerImpl::audit(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr)
{
    log(5, message, &sr);
}
void LoggerImpl::audit(std::string const& message,
                       cppmicroservices::ServiceReferenceBase const& sr,
                       const std::exception_ptr ex)
{
    log(5, message, &sr, &ex);
}

// Debug methods
void LoggerImpl::debug(std::string const& message)
{
    log(3, message);
}
void LoggerImpl::debug(std::string const& format, std::string const& arg)
{
    log(3, format, arg);
}
void LoggerImpl::debug(std::string const& format, std::string const& arg1, std::string const& arg2)
{
    log(3, format, arg1, arg2);
}
void LoggerImpl::debug(std::string const& message, const std::exception_ptr ex)
{
    log(3, message, nullptr, &ex);
}
void LoggerImpl::debug(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr)
{
    log(3, message, &sr);
}
void LoggerImpl::debug(std::string const& message,
                       cppmicroservices::ServiceReferenceBase const& sr,
                       const std::exception_ptr ex)
{
    log(3, message, &sr, &ex);
}

// Error methods
void LoggerImpl::error(std::string const& message)
{
    log(0, message);
}
void LoggerImpl::error(std::string const& format, std::string const& arg)
{
    log(0, format, arg);
}
void LoggerImpl::error(std::string const& format, std::string const& arg1, std::string const& arg2)
{
    log(0, format, arg1, arg2);
}
void LoggerImpl::error(std::string const& message, const std::exception_ptr ex)
{
    log(0, message, nullptr, &ex);
}
void LoggerImpl::error(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr)
{
    log(0, message, &sr);
}
void LoggerImpl::error(std::string const& message,
                       cppmicroservices::ServiceReferenceBase const& sr,
                       const std::exception_ptr ex)
{
    log(0, message, &sr, &ex);
}

// Info methods
void LoggerImpl::info(std::string const& message)
{
    log(2, message);
}
void LoggerImpl::info(std::string const& format, std::string const& arg)
{
    log(2, format, arg);
}
void LoggerImpl::info(std::string const& format, std::string const& arg1, std::string const& arg2)
{
    log(2, format, arg1, arg2);
}
void LoggerImpl::info(std::string const& message, const std::exception_ptr ex)
{
    log(2, message, nullptr, &ex);
}
void LoggerImpl::info(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr)
{
    log(2, message, &sr);
}
void LoggerImpl::info(std::string const& message,
                      cppmicroservices::ServiceReferenceBase const& sr,
                      const std::exception_ptr ex)
{
    log(2, message, &sr, &ex);
}

// Trace methods
void LoggerImpl::trace(std::string const& message)
{
    log(4, message);
}
void LoggerImpl::trace(std::string const& format, std::string const& arg)
{
    log(4, format, arg);
}
void LoggerImpl::trace(std::string const& format, std::string const& arg1, std::string const& arg2)
{
    log(4, format, arg1, arg2);
}
void LoggerImpl::trace(std::string const& message, const std::exception_ptr ex)
{
    log(4, message, nullptr, &ex);
}
void LoggerImpl::trace(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr)
{
    log(4, message, &sr);
}
void LoggerImpl::trace(std::string const& message,
                       cppmicroservices::ServiceReferenceBase const& sr,
                       const std::exception_ptr ex)
{
    log(4, message, &sr, &ex);
}

// Warn methods
void LoggerImpl::warn(std::string const& message)
{
    log(1, message);
}
void LoggerImpl::warn(std::string const& format, std::string const& arg)
{
    log(1, format, arg);
}
void LoggerImpl::warn(std::string const& format, std::string const& arg1, std::string const& arg2)
{
    log(1, format, arg1, arg2);
}
void LoggerImpl::warn(std::string const& message, const std::exception_ptr ex)
{
    log(1, message, nullptr, &ex);
}
void LoggerImpl::warn(std::string const& message, cppmicroservices::ServiceReferenceBase const& sr)
{
    log(1, message, &sr);
}
void LoggerImpl::warn(std::string const& message,
                      cppmicroservices::ServiceReferenceBase const& sr,
                      const std::exception_ptr ex)
{
    log(1, message, &sr, &ex);
}

// LogServiceImpl implementation

LogServiceImpl::LogServiceImpl()
    : m_logTextEdit(nullptr)
{
    initSpdlog();
}

LogServiceImpl::~LogServiceImpl()
{
    spdlog::shutdown();
}

void LogServiceImpl::initSpdlog()
{
    // 获取日志目录（应用程序目录下的 logs）
    QString logDir = QCoreApplication::applicationDirPath() + "/logs";
    QDir dir(logDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    // 创建按天分割的滚动文件 logger
    // 文件名格式：logs/log_yyyy-MM-dd.txt
    QString todayFile = logDir + "/log_" + QDate::currentDate().toString("yyyy-MM-dd") + ".txt";
    
    try {
        // 文件 logger - 使用大小滚动作为后备（每个文件最大 10MB）
        m_fileLogger = spdlog::rotating_logger_mt(
            "file_logger",
            todayFile.toStdString(),
            10 * 1024 * 1024,  // 10MB
            30                 // 保留 30 个文件
        );
        m_fileLogger->set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] [%n] %v");
        m_fileLogger->set_level(spdlog::level::trace);
        
        spdlog::info("File logger initialized: {}", todayFile.toStdString());
    }
    catch (const spdlog::spdlog_ex& ex) {
        spdlog::error("File logger init failed: {}", ex.what());
    }
    
    // 注意：console logger 会在 setLogWidget 中创建
}

void LogServiceImpl::setLogWidget(LogWidget* widget)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logWidget = widget;
    
    // 如果 LogWidget 内部有 QTextEdit，可以创建 qt sink
    // 这里暂时只使用文件 logger
}

LogWidget* LogServiceImpl::getLogWidget() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_logWidget;
}

std::shared_ptr<cppmicroservices::logservice::Logger>
LogServiceImpl::getLogger(std::string const& name) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return std::make_shared<LoggerImpl>(name, const_cast<LogServiceImpl*>(this));
}

std::shared_ptr<cppmicroservices::logservice::Logger>
LogServiceImpl::getLogger(const cppmicroservices::Bundle& bundle, std::string const& name) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return std::make_shared<LoggerImpl>(name, bundle, const_cast<LogServiceImpl*>(this));
}

void LogServiceImpl::Log(cppmicroservices::logservice::SeverityLevel level,
                         std::string const& message)
{
    int lvl            = severityToLevel(level);
    QString bundleName = "system";
    QString msg        = QString::fromStdString(message);

    addLogEntry(lvl, bundleName, msg);
}

void LogServiceImpl::Log(cppmicroservices::logservice::SeverityLevel level,
                         std::string const& message,
                         const std::exception_ptr ex)
{
    int lvl            = severityToLevel(level);
    QString bundleName = "system";
    QString msg        = QString::fromStdString(message);

    if (ex) {
        try {
            std::rethrow_exception(ex);
        }
        catch (const std::exception& e) {
            msg += " - Exception: " + QString::fromStdString(e.what());
        }
        catch (...) {
            msg += " - Unknown exception";
        }
    }

    addLogEntry(lvl, bundleName, msg);
}

void LogServiceImpl::Log(cppmicroservices::ServiceReferenceBase const& sr,
                         cppmicroservices::logservice::SeverityLevel level,
                         std::string const& message)
{
    int lvl = severityToLevel(level);
    QString bundleName =
        sr.GetBundle() ? QString::fromStdString(sr.GetBundle().GetSymbolicName()) : "unknown";
    QString msg = QString::fromStdString(message);

    addLogEntry(lvl, bundleName, msg);
}

void LogServiceImpl::Log(cppmicroservices::ServiceReferenceBase const& sr,
                         cppmicroservices::logservice::SeverityLevel level,
                         std::string const& message,
                         const std::exception_ptr ex)
{
    int lvl = severityToLevel(level);
    QString bundleName =
        sr.GetBundle() ? QString::fromStdString(sr.GetBundle().GetSymbolicName()) : "unknown";
    QString msg = QString::fromStdString(message);

    if (ex) {
        try {
            std::rethrow_exception(ex);
        }
        catch (const std::exception& e) {
            msg += " - Exception: " + QString::fromStdString(e.what());
        }
        catch (...) {
            msg += " - Unknown exception";
        }
    }

    addLogEntry(lvl, bundleName, msg);
}

void LogServiceImpl::addLogEntry(int level, const QString& bundleName, const QString& message)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // UI 更新
    if (m_logWidget) {
        QMetaObject::invokeMethod(
            m_logWidget,
            [this, level, bundleName, message]() {
                m_logWidget->addLog(level, bundleName, message);
            },
            Qt::QueuedConnection);
    }
    
    // spdlog 输出（自动异步，线程安全）
    if (m_fileLogger) {
        spdlog::source_loc source{};
        spdlog::level::level_enum spdLevel = static_cast<spdlog::level::level_enum>(level);
        
        m_fileLogger->log(source, spdLevel, "[{}] {}", bundleName.toStdString(), message.toStdString());
    }
}

int LogServiceImpl::severityToLevel(cppmicroservices::logservice::SeverityLevel level) const
{
    switch (level) {
        case cppmicroservices::logservice::SeverityLevel::LOG_ERROR: return 0;
        case cppmicroservices::logservice::SeverityLevel::LOG_WARNING: return 1;
        case cppmicroservices::logservice::SeverityLevel::LOG_INFO: return 2;
        case cppmicroservices::logservice::SeverityLevel::LOG_DEBUG: return 3;
        default: return 2;
    }
}

QString LogServiceImpl::getBundleName(const cppmicroservices::Bundle& bundle) const
{
    if (!bundle)
        return "system";
    return QString::fromStdString(bundle.GetSymbolicName());
}

