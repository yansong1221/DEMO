#pragma once

#include "PluginBundleTableModel.h"
#include "TaskServiceTableModel.h"

#include <QMainWindow>
#include <cppmicroservices/Framework.h>
#include <memory>

#include "demo/IGreetingService.h"
#include "demo/IWidgetService.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class PluginBundleActionDelegate;
class TaskServiceActionDelegate;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onBrowsePluginFolder();
    void onRefreshBundleList();
    void onLoadBundleRow(int row);
    void onUnloadBundleRow(int row);
    void onBundleTableSelectionChanged();
    void onRefreshTaskServices();
    void onTaskServiceSelectionChanged();
    void onTaskServiceStartStopRequested(int row);
    void onTaskServiceConfigRequested(int row);
    void onTaskServiceLog(QString const& message);

private:
    void setupModels();
    void setupViews();
    void setupConnections();
    void appendLog(const QString& line);
    int currentTaskServiceRow() const;
    std::shared_ptr<demo::ITaskService::IBasicConfig> buildTaskServiceConfig(int row,
                                                                             bool* ok) const;
    void startTaskService(int row);
    void stopTaskService(int row);

    Ui::MainWindow* ui;
    cppmicroservices::Framework m_framework;
    PluginBundleTableModel* m_bundleModel                  = nullptr;
    TaskServiceTableModel* m_taskServiceModel              = nullptr;
    PluginBundleActionDelegate* m_actionDelegate           = nullptr;
    TaskServiceActionDelegate* m_taskServiceActionDelegate = nullptr;

    struct PluginState
    {
        cppmicroservices::ServiceReference<demo::IWidgetService> ref;
        std::shared_ptr<demo::IWidgetService> service;
        QDockWidget* dock_w = nullptr;
    };
    std::vector<PluginState> m_Plugins;
   cppmicroservices::ListenerToken  m_ListenerToken;
};
