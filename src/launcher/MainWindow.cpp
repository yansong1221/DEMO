#include "MainWindow.h"
#include "bundle/BundleManagerDockWidget.h"
#include "log/LogServiceImpl.h"
#include "log/LogWidget.h"
#include "task/TaskServiceDockWidget.h"

#include <common/Logger.h>

#include <QCloseEvent>
#include <QCoreApplication>
#include <QVBoxLayout>
#include <QWidget>


#include <cppmicroservices/FrameworkFactory.h>
#include <cppmicroservices/logservice/LogService.hpp>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QMenu>
#include <QMenuBar>
#include <chrono>

MainWindow::MainWindow(cppmicroservices::BundleContext bundleContext, QWidget* parent)
    : KDDockWidgets::QtWidgets::MainWindow(
          QStringLiteral("MainWindow"), KDDockWidgets::MainWindowOption_HasCentralWidget, parent)
    , m_bundleContext(bundleContext)
{
    setupUI();
    setupLogService();

    {
        QDir dir(QCoreApplication::applicationDirPath());

        QStringList filters;
        filters << "*.dll";

        for (const QFileInfo& fileInfo : dir.entryInfoList(filters, QDir::Files)) {
            try {
                m_bundleContext.InstallBundles(fileInfo.absoluteFilePath().toStdString());
            }
            catch (...) {
            }
        }

        for (auto bundle : m_bundleContext.GetBundles())
            bundle.Start();
    }
    common::Logger::init(m_bundleContext);

    setupServiceListener();
    setupDockWidgets();

    // 使用 Logger 输出启动日志（无需传入 context）
    common::Logger::info("框架已启动，支持 Bundle 管理和 ITaskService 服务发现。");
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    // 创建 LogWidget 作为中心控件
    m_logWidget = new LogWidget(this);

    setPersistentCentralWidget(m_logWidget);

    auto menubar = menuBar();
    m_toggleMenu = new QMenu("Toggle", this);
    menubar->addMenu(m_toggleMenu);
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

    // 首次启动自动加载所有 bundles
    m_bundleManagerDock->refreshBundleList();
    int count = m_bundleManagerDock->bundleCount();
    for (int i = 0; i < count; ++i) {
        m_bundleManagerDock->loadBundle(i);
    }
}

void MainWindow::setupServiceListener()
{
    m_ListenerToken =
        m_bundleContext.AddServiceListener([this](cppmicroservices::ServiceEvent const& evt) {
            auto type     = evt.GetType();
            auto eventRef = evt.GetServiceReference();

            if (type & cppmicroservices::ServiceEvent::SERVICE_REGISTERED) {
                auto service = m_bundleContext.GetService<IWidgetService>(eventRef);
                if (!service) {
                    return;
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
                p.ref     = eventRef;
                p.dock_w  = dock_W;
                m_Plugins.push_back(p);
            }
            else if (type & cppmicroservices::ServiceEvent::SERVICE_UNREGISTERING) {
                for (size_t i = 0; i < m_Plugins.size(); ++i) {
                    const cppmicroservices::ServiceReferenceBase currentRef = m_Plugins[i].ref;
                    auto dock_w                                             = m_Plugins[i].dock_w;

                    if (currentRef == eventRef) {
                        dock_w->setWidget(nullptr);
                        delete dock_w;
                        m_Plugins.erase(m_Plugins.begin() + i);
                        break;
                    }
                }
            }
        });
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    for (auto bundle : m_bundleContext.GetBundles()) {
        if (bundle == m_bundleContext.GetBundle())
            continue;
        auto s = bundle.GetSymbolicName();
        qDebug() << QString::fromStdString(s);
    }
    for (auto bundle : m_bundleContext.GetBundles()) {
        if (bundle == m_bundleContext.GetBundle())
            continue;
        auto s = bundle.GetSymbolicName();
        qDebug() << QString::fromStdString(s);
        bundle.Stop();
    }
    // for (auto bundle : m_bundleContext.GetBundles()) {
    //     if (bundle == m_bundleContext.GetBundle())
    //         continue;
    //     auto s = bundle.GetSymbolicName();
    //     bundle.Stop();
    // }

    m_Plugins.clear();
    m_bundleContext.RemoveListener(std::move(m_ListenerToken));

    common::Logger::reset();

    KDDockWidgets::QtWidgets::MainWindow::closeEvent(event);
}
