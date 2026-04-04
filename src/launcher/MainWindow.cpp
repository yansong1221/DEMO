#include "MainWindow.h"
#include "BundleManager/BundleManagerDockWidget.h"
#include "LogService/LogServiceImpl.h"

#include <common/Logger.h>

#include <QCloseEvent>
#include <QCoreApplication>
#include <QVBoxLayout>
#include <QWidget>

#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/logservice/LogService.hpp>

#include "MainImGuiWidget.h"
#include "cppmicroservices/ServiceEvent.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QMenu>
#include <QMenuBar>
#include <QTimer>
#include <chrono>

MainWindow::MainWindow(cppmicroservices::BundleContext bundleContext, QWidget* parent)
    : KDDockWidgets::QtWidgets::MainWindow(QStringLiteral("MainWindow"),
                                           KDDockWidgets::MainWindowOption_HasCentralGroup,
                                           parent)
    , m_bundleContext(bundleContext)
    , m_Tracker(bundleContext, this)
{
    setupUI();
    setupLogService();
    common::Log::init(m_bundleContext);

    {
#ifdef Q_OS_WIN
        const QStringList filters { QStringLiteral("*.dll") };
#else
        const QStringList filters { QStringLiteral("*.so") };
#endif
        QDir dir(QCoreApplication::applicationDirPath());
        auto files = dir.entryInfoList(filters, QDir::Files | QDir::Readable, QDir::Name);
        for (auto const& file : files)
        {
            try
            {
                m_bundleContext.InstallBundles(file.absoluteFilePath().toStdString());
                common::Log::info("Found plugin file: {}", file.fileName().toStdString());
            }
            catch (...)
            {
            }
        }
        m_Bundles = m_bundleContext.GetBundles();
        for (auto& bundle : m_Bundles)
        {
            bundle.Start();
        }
    }

    setupDockWidgets();
}

MainWindow::~MainWindow() { m_Tracker.Close(); }

void
MainWindow::setupUI()
{
    auto menubar = menuBar();
    m_toggleMenu = new QMenu("Toggle", this);
    menubar->addMenu(m_toggleMenu);

    // 创建 LogWidget 作为中心控件
    m_MainImGui = new MainImGuiWidget(m_bundleContext, this);
    auto dock = new KDDockWidgets::QtWidgets::DockWidget("MainImGui");
    dock->setWidget(m_MainImGui);

    addDockWidget(dock, KDDockWidgets::Location_OnLeft);
    dock->show();
    m_toggleMenu->addAction(dock->toggleAction());

    // setPersistentCentralWidget(m_logWidget);
}

void
MainWindow::setupLogService()
{
    // 创建 LogService 实现
    m_logServiceImpl = std::make_shared<LogServiceImpl>();
    m_logServiceImpl->Init();

    // 注册 LogService 到框架
    cppmicroservices::ServiceProperties props;
    props["service.description"] = std::string("Qt-based LogService Implementation");
    props["service.vendor"] = std::string("CppMicroServices Demo");

    m_bundleContext.RegisterService<cppmicroservices::logservice::LogService>(m_logServiceImpl, props);
    m_bundleContext.RegisterService<cppmicroservices::logservice::LoggerFactory>(m_logServiceImpl, props);
    m_bundleContext.RegisterService<service::IImGuiDrawService>(m_logServiceImpl, props);
    // 记录启动日志
    m_logServiceImpl->Log(cppmicroservices::logservice::SeverityLevel::LOG_INFO, "LogService registered successfully");
}

void
MainWindow::setupDockWidgets()
{
    // 创建 Bundle 管理 DockWidget
    m_bundleManagerDock = new BundleManagerDockWidget(m_bundleContext, this);
    addDockWidget(m_bundleManagerDock, KDDockWidgets::Location_OnLeft);
    m_bundleManagerDock->show();
    m_toggleMenu->addAction(m_bundleManagerDock->toggleAction());

    m_Tracker.Open();

    QTimer::singleShot(0,
                       this,
                       [this]()
                       {
                           // 首次启动自动加载所有 bundles
                           m_bundleManagerDock->refreshBundleList();
                           int count = m_bundleManagerDock->bundleCount();
                           for (int i = 0; i < count; ++i)
                           {
                               m_bundleManagerDock->loadBundle(i);
                           }
                       });
}
void
MainWindow::closeEvent(QCloseEvent* event)
{
    for (auto& bundle : m_bundleContext.GetBundles())
    {
        if (bundle == m_bundleContext.GetBundle())
        {
            continue;
        }
        bundle.Stop();
    }

    m_Tracker.Close();

    m_Plugins.clear();

    common::Log::reset();

    KDDockWidgets::QtWidgets::MainWindow::closeEvent(event);
}

std::shared_ptr<service::IWidgetService>
MainWindow::AddingService(cppmicroservices::ServiceReference<service::IWidgetService> const& reference)
{
    auto service = m_bundleContext.GetService(reference);
    if (!service)
    {
        return nullptr;
    }

    PluginState p;
    p.service = service;
    p.ref = reference;

    if (auto w = service->widget(); w)
    {
        auto dock = new KDDockWidgets::QtWidgets::DockWidget(QString::fromStdString(service->uniqueName()));
        dock->setWidget(w);
        dock->setTitle(w->windowTitle());
        m_toggleMenu->addAction(dock->toggleAction());
        addDockWidgetAsTab(dock);
        dock->show();
        p.dock = dock;
    }
    m_Plugins.push_back(p);
    return service;
}

void
MainWindow::ModifiedService(cppmicroservices::ServiceReference<service::IWidgetService> const& reference,
                            std::shared_ptr<service::IWidgetService> const& service)
{
}

void
MainWindow::RemovedService(cppmicroservices::ServiceReference<service::IWidgetService> const& reference,
                           std::shared_ptr<service::IWidgetService> const& service)
{
    for (size_t i = 0; i < m_Plugins.size(); ++i)
    {
        if (m_Plugins[i].ref == reference)
        {
            if (auto dock = m_Plugins[i].dock; dock)
            {
                dock->setWidget(nullptr);
                delete dock;
            }
            m_Plugins.erase(m_Plugins.begin() + i);
            break;
        }
    }
}
