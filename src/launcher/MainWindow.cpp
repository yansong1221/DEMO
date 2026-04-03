#include "MainWindow.h"
#include "BundleManager/BundleManagerDockWidget.h"
#include "LogService/LogServiceImpl.h"
#include "LogService/LogWidget.h"
#include "TaskServiceManager/TaskServiceDockWidget.h"

#include <common/Logger.h>

#include <QCloseEvent>
#include <QCoreApplication>
#include <QVBoxLayout>
#include <QWidget>


#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/logservice/LogService.hpp>

#include "cppmicroservices/ServiceEvent.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QMenu>
#include <QMenuBar>
#include <QTimer>
#include <chrono>

MainWindow::MainWindow(cppmicroservices::BundleContext bundleContext, QWidget* parent)
    : KDDockWidgets::QtWidgets::MainWindow(
          QStringLiteral("MainWindow"), KDDockWidgets::MainWindowOption_HasCentralGroup, parent)
    , m_bundleContext(bundleContext)
    , m_Tracker(bundleContext, this)
{
    setupUI();
    setupLogService();
    common::Log::init(m_bundleContext);


    {
#ifdef Q_OS_WIN
        const QStringList filters {QStringLiteral("*.dll")};
#else
        const QStringList filters {QStringLiteral("*.so")};
#endif
        QDir dir(QCoreApplication::applicationDirPath());
        auto files = dir.entryInfoList(filters, QDir::Files | QDir::Readable, QDir::Name);
        for (const auto& file : files) {
            try {
                m_bundleContext.InstallBundles(file.absoluteFilePath().toStdString());
                qDebug() << "Found plugin file:" << file.fileName();
            }
            catch (...) {
            }
        }
        m_Bundles = m_bundleContext.GetBundles();
        for (auto bundle : m_Bundles)
            bundle.Start();
    }

    setupDockWidgets();

    // 使用 Logger 输出启动日志（无需传入 context）
    common::Log::info("框架已启动，支持 Bundle 管理和 ITaskService 服务发现。");
}

MainWindow::~MainWindow()
{
    m_Tracker.Close();
}

void MainWindow::setupUI()
{
    auto menubar = menuBar();
    m_toggleMenu = new QMenu("Toggle", this);
    menubar->addMenu(m_toggleMenu);

    // 创建 LogWidget 作为中心控件
    m_logWidget = new LogWidget(this);

    auto dock = new KDDockWidgets::QtWidgets::DockWidget("LogWidget");
    dock->setWidget(m_logWidget);

    addDockWidget(dock, KDDockWidgets::Location_OnLeft);
    dock->show();
    m_toggleMenu->addAction(dock->toggleAction());


    // setPersistentCentralWidget(m_logWidget);
}

void MainWindow::setupLogService()
{
    // 创建 LogService 实现
    m_logServiceImpl = std::make_shared<LogServiceImpl>(m_logWidget);

    // 注册 LogService 到框架
    cppmicroservices::ServiceProperties props;
    props["service.description"] = std::string("Qt-based LogService Implementation");
    props["service.vendor"]      = std::string("CppMicroServices Demo");

    m_bundleContext.RegisterService<cppmicroservices::logservice::LogService>(m_logServiceImpl,
                                                                              props);
    m_bundleContext.RegisterService<cppmicroservices::logservice::LoggerFactory>(m_logServiceImpl,
                                                                                 props);
    // 记录启动日志
    m_logServiceImpl->Log(cppmicroservices::logservice::SeverityLevel::LOG_INFO,
                          "LogService registered successfully");
}

void MainWindow::setupDockWidgets()
{
    // 创建 Bundle 管理 DockWidget
    m_bundleManagerDock = new BundleManagerDockWidget(m_bundleContext, this);
    addDockWidget(m_bundleManagerDock, KDDockWidgets::Location_OnLeft);
    m_bundleManagerDock->show();
    m_toggleMenu->addAction(m_bundleManagerDock->toggleAction());


    // 创建任务服务 DockWidget
    m_taskServiceDock = new TaskServiceDockWidget(m_bundleContext, this);
    // addDockWidget(m_taskServiceDock, KDDockWidgets::Location_OnBottom, m_bundleManagerDock);
    m_bundleManagerDock->addDockWidgetAsTab(m_taskServiceDock);
    m_taskServiceDock->show();
    m_toggleMenu->addAction(m_taskServiceDock->toggleAction());


    m_Tracker.Open();

    QTimer::singleShot(0, this, [this]() {
        // 首次启动自动加载所有 bundles
        m_bundleManagerDock->refreshBundleList();
        int count = m_bundleManagerDock->bundleCount();
        for (int i = 0; i < count; ++i) {
            m_bundleManagerDock->loadBundle(i);
        }
    });
}
void MainWindow::closeEvent(QCloseEvent* event)
{
    for (auto& bundle : m_bundleContext.GetBundles()) {
        if (bundle == m_bundleContext.GetBundle())
            continue;
        bundle.Stop();
    }

    m_Tracker.Close();

    m_Plugins.clear();

    common::Log::reset();

    KDDockWidgets::QtWidgets::MainWindow::closeEvent(event);
}

std::shared_ptr<service::IWidgetService> MainWindow::AddingService(
    cppmicroservices::ServiceReference<service::IWidgetService> const& reference)
{
    auto service = m_bundleContext.GetService(reference);
    if (!service) {
        return nullptr;
    }
    auto w = service->widget();

    auto dock_W = new KDDockWidgets::QtWidgets::DockWidget(service->uniqueName());
    dock_W->setWidget(w);
    dock_W->setTitle(w->windowTitle());
    m_toggleMenu->addAction(dock_W->toggleAction());

    addDockWidgetAsTab(dock_W);
    dock_W->show();

    PluginState p;
    p.service = service;
    p.ref     = reference;
    p.dock_w  = dock_W;
    m_Plugins.push_back(p);
    return service;
}

void MainWindow::ModifiedService(
    cppmicroservices::ServiceReference<service::IWidgetService> const& reference,
    std::shared_ptr<service::IWidgetService> const& service)
{
}

void MainWindow::RemovedService(
    cppmicroservices::ServiceReference<service::IWidgetService> const& reference,
    std::shared_ptr<service::IWidgetService> const& service)
{
    for (size_t i = 0; i < m_Plugins.size(); ++i) {
        auto dock_w = m_Plugins[i].dock_w;
        if (m_Plugins[i].ref == reference) {
            dock_w->setWidget(nullptr);
            delete dock_w;
            m_Plugins.erase(m_Plugins.begin() + i);
            break;
        }
    }
}
