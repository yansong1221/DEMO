#pragma once

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <kddockwidgets/qtwidgets/views/MainWindow.h>

#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/ListenerToken.h>
#include <cppmicroservices/ServiceReference.h>
#include <memory>

#include "cppmicroservices/ServiceTracker.h"
#include "cppmicroservices/ServiceTrackerCustomizer.h"
#include <service/ITaskService.h>
#include <service/IWidgetService.h>

class BundleManagerDockWidget;
class TaskServiceDockWidget;
class LogWidget;
class LogServiceImpl;

class MainWindow
    : public KDDockWidgets::QtWidgets::MainWindow
    , public cppmicroservices::ServiceTrackerCustomizer<service::IWidgetService>
{
    Q_OBJECT

  public:
    explicit MainWindow(cppmicroservices::BundleContext bundleContext, QWidget* parent = nullptr);
    ~MainWindow() override;

  protected:
    void closeEvent(QCloseEvent* event) override;

  protected:
    std::shared_ptr<service::IWidgetService> AddingService(
        cppmicroservices::ServiceReference<service::IWidgetService> const& reference) override;

    void ModifiedService(cppmicroservices::ServiceReference<service::IWidgetService> const& reference,
                         std::shared_ptr<service::IWidgetService> const& service) override;

    void RemovedService(cppmicroservices::ServiceReference<service::IWidgetService> const& reference,
                        std::shared_ptr<service::IWidgetService> const& service) override;

  private:
    void setupUI();
    void setupDockWidgets();
    void setupLogService();

    cppmicroservices::BundleContext m_bundleContext;
    cppmicroservices::ServiceTracker<service::IWidgetService> m_Tracker;

    QMenu* m_toggleMenu = nullptr;

    // Dock widgets
    BundleManagerDockWidget* m_bundleManagerDock = nullptr;
    TaskServiceDockWidget* m_taskServiceDock = nullptr;

    // Log widget (persistent central widget)
    LogWidget* m_logWidget = nullptr;

    // LogService implementation
    std::shared_ptr<LogServiceImpl> m_logServiceImpl;

    struct PluginState
    {
        cppmicroservices::ServiceReference<service::IWidgetService> ref;
        std::shared_ptr<service::IWidgetService> service;
        KDDockWidgets::QtWidgets::DockWidget* dock_w = nullptr;
    };
    std::vector<PluginState> m_Plugins;
    std::vector<cppmicroservices::Bundle> m_Bundles;
};
