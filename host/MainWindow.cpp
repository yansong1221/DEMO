#include "MainWindow.h"
#include "bundle/BundleManagerDockWidget.h"
#include "task/TaskServiceDockWidget.h"
#include "log/LogWidget.h"
#include "log/LogServiceImpl.h"

#include <QCloseEvent>
#include <QCoreApplication>
#include <QVBoxLayout>
#include <QWidget>

#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/logservice/LogService.hpp>

#include <chrono>

MainWindow::MainWindow(QWidget* parent)
    : KDDockWidgets::QtWidgets::MainWindow(
          QStringLiteral("MainWindow"), KDDockWidgets::MainWindowOption_HasCentralWidget, parent)
    , m_framework(cppmicroservices::FrameworkFactory {}.NewFramework())
{
    m_framework.Init();
    m_framework.Start();

    setupUI();
    setupLogService();
    setupDockWidgets();
    setupConnections();
    setupServiceListener();

    appendLog(tr("框架已启动，支持 Bundle 管理和 ITaskService 服务发现。"));
}

MainWindow::~MainWindow()
{
}

LogServiceImpl* MainWindow::getLogServiceImpl() const
{
    return m_logServiceImpl.get();
}

void MainWindow::setupUI()
{
    // 创建 LogWidget 作为中心控件
    m_logWidget = new LogWidget(this);
    
    setPersistentCentralWidget(m_logWidget);
}

void MainWindow::setupLogService()
{
    // 创建 LogService 实现
    m_logServiceImpl = std::make_shared<LogServiceImpl>();
    
    // 设置日志显示控件
    m_logServiceImpl->setLogWidget(m_logWidget);
    
    // 注册 LogService 到框架
    cppmicroservices::ServiceProperties props;
    props["service.description"] = std::string("Qt-based LogService Implementation");
    props["service.vendor"] = std::string("CppMicroServices Demo");
    
    m_framework.GetBundleContext().RegisterService<cppmicroservices::logservice::LogService>(
        m_logServiceImpl, props);
    m_framework.GetBundleContext().RegisterService<cppmicroservices::logservice::LoggerFactory>(
        m_logServiceImpl, props);
    
    // 记录启动日志
    m_logServiceImpl->Log(cppmicroservices::logservice::SeverityLevel::LOG_INFO, 
                          "LogService registered successfully");
}

void MainWindow::setupDockWidgets()
{
    // 创建 Bundle 管理 DockWidget
    m_bundleManagerDock = new BundleManagerDockWidget(&m_framework, this);
    m_bundleManagerDock->setPluginDir(QCoreApplication::applicationDirPath());
    addDockWidget(m_bundleManagerDock, KDDockWidgets::Location_OnLeft);
    m_bundleManagerDock->show();

    // 创建任务服务 DockWidget
    m_taskServiceDock = new TaskServiceDockWidget(&m_framework, this);
    addDockWidget(m_taskServiceDock, KDDockWidgets::Location_OnBottom, m_bundleManagerDock);
    m_taskServiceDock->show();
}

void MainWindow::setupConnections()
{
    // 连接日志信号
    connect(
        m_bundleManagerDock, &BundleManagerDockWidget::logMessage, this, &MainWindow::onLogMessage);
    connect(m_taskServiceDock, &TaskServiceDockWidget::logMessage, this, &MainWindow::onLogMessage);
}

void MainWindow::setupServiceListener()
{
    m_ListenerToken = m_framework.GetBundleContext().AddServiceListener(
        [this](cppmicroservices::ServiceEvent const& evt) {
            auto type     = evt.GetType();
            auto eventRef = evt.GetServiceReference();

            if (type & cppmicroservices::ServiceEvent::SERVICE_REGISTERED) {
                auto ctx     = m_framework.GetBundleContext();
                auto service = ctx.GetService<demo::IWidgetService>(eventRef);
                if (!service) {
                    return;
                }
                auto w = service->widget();

                auto dock_W = new KDDockWidgets::QtWidgets::DockWidget(service->uniqueName());
                dock_W->setWidget(w);

                addDockWidget(dock_W, KDDockWidgets::Location_OnRight);
                dock_W->show();

                PluginState p;
                p.service = service;
                p.ref     = eventRef;
                p.dock_w  = dock_W;
                m_Plugins.push_back(p);
            }
            else if (type & cppmicroservices::ServiceEvent::SERVICE_UNREGISTERING) {
                for (size_t i = 0; i < m_Plugins.size(); ++i) {
                    const cppmicroservices::ServiceReferenceBase currentRef = m_Plugins[i].ref;
                    auto dock_w                                             = m_Plugins[i].dock_w;

                    if (currentRef == eventRef) {
                        if (dock_w) {
                            dock_w->close();
                            dock_w->deleteLater();
                        }
                        m_Plugins.erase(m_Plugins.begin() + i);
                        break;
                    }
                }
            }
        });
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    // 清理插件 dock widgets
    for (auto& plugin : m_Plugins) {
        if (plugin.dock_w) {
            plugin.dock_w->close();
            plugin.dock_w->deleteLater();
        }
    }
    m_Plugins.clear();

    for (auto& bundle : m_framework.GetBundleContext().GetBundles()) {
        bundle.Stop();
    }

    m_framework.GetBundleContext().RemoveListener(std::move(m_ListenerToken));

    m_framework.Stop();
    m_framework.WaitForStop((std::chrono::milliseconds::max)());

    KDDockWidgets::QtWidgets::MainWindow::closeEvent(event);
}

void MainWindow::appendLog(const QString& line)
{
    if (m_logWidget) {
        m_logWidget->addLog(2, "host_app", line); // 2 = INFO level
    }
}

void MainWindow::onLogMessage(const QString& message)
{
    appendLog(message);
}
