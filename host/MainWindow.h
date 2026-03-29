#pragma once

#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <kddockwidgets/qtwidgets/views/MainWindow.h>

#include <cppmicroservices/Framework.h>
#include <cppmicroservices/FrameworkEvent.h>
#include <cppmicroservices/ServiceReference.h>
#include <cppmicroservices/ListenerToken.h>
#include <QTextEdit>
#include <memory>

#include "demo/IGreetingService.h"
#include "demo/IWidgetService.h"

class BundleManagerDockWidget;
class TaskServiceDockWidget;

class MainWindow : public KDDockWidgets::QtWidgets::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onLogMessage(const QString& message);

private:
    void setupUI();
    void setupDockWidgets();
    void setupConnections();
    void setupServiceListener();
    void appendLog(const QString& line);

    cppmicroservices::Framework m_framework;
    
    // Dock widgets
    BundleManagerDockWidget* m_bundleManagerDock = nullptr;
    TaskServiceDockWidget* m_taskServiceDock = nullptr;
    
    // Log widget (persistent central widget)
    QTextEdit* m_logEdit = nullptr;

    struct PluginState
    {
        cppmicroservices::ServiceReference<demo::IWidgetService> ref;
        std::shared_ptr<demo::IWidgetService> service;
        KDDockWidgets::QtWidgets::DockWidget* dock_w = nullptr;
    };
    std::vector<PluginState> m_Plugins;
    cppmicroservices::ListenerToken m_ListenerToken;
};
