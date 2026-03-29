#pragma once

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <kddockwidgets/qtwidgets/views/MainWindow.h>

#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/ListenerToken.h>
#include <cppmicroservices/ServiceReference.h>
#include <memory>

#include <service/IGreetingService.h>
#include <service/IWidgetService.h>

using service::IWidgetService;

class BundleManagerDockWidget;
class TaskServiceDockWidget;
class LogWidget;
class LogServiceImpl;

class MainWindow : public KDDockWidgets::QtWidgets::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void setupUI();
    void setupDockWidgets();
    void setupServiceListener();
    void setupLogService();

    cppmicroservices::Framework m_framework;

    // Dock widgets
    BundleManagerDockWidget* m_bundleManagerDock = nullptr;
    TaskServiceDockWidget* m_taskServiceDock     = nullptr;

    // Log widget (persistent central widget)
    LogWidget* m_logWidget = nullptr;

    // LogService implementation
    std::shared_ptr<LogServiceImpl> m_logServiceImpl;

    struct PluginState
    {
        cppmicroservices::ServiceReference<IWidgetService> ref;
        std::shared_ptr<IWidgetService> service;
        KDDockWidgets::QtWidgets::DockWidget* dock_w = nullptr;
    };
    std::vector<PluginState> m_Plugins;
    cppmicroservices::ListenerToken m_ListenerToken;
};
