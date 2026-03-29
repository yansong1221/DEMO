#pragma once

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <kddockwidgets/qtwidgets/views/MainWindow.h>

#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/ServiceReference.h>
#include <cppmicroservices/ListenerToken.h>
#include <memory>

#include "demo/IGreetingService.h"
#include "demo/IWidgetService.h"

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

    // 获取 LogServiceImpl 实例（供其他组件使用）
    LogServiceImpl* getLogServiceImpl() const;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onLogMessage(const QString& message);

private:
    void setupUI();
    void setupDockWidgets();
    void setupConnections();
    void setupServiceListener();
    void setupLogService();
    void appendLog(const QString& line);

    cppmicroservices::Framework m_framework;
    
    // Dock widgets
    BundleManagerDockWidget* m_bundleManagerDock = nullptr;
    TaskServiceDockWidget* m_taskServiceDock = nullptr;
    
    // Log widget (persistent central widget)
    LogWidget* m_logWidget = nullptr;
    
    // LogService implementation
    std::shared_ptr<LogServiceImpl> m_logServiceImpl;

    struct PluginState
    {
        cppmicroservices::ServiceReference<demo::IWidgetService> ref;
        std::shared_ptr<demo::IWidgetService> service;
        KDDockWidgets::QtWidgets::DockWidget* dock_w = nullptr;
    };
    std::vector<PluginState> m_Plugins;
    cppmicroservices::ListenerToken m_ListenerToken;
};
