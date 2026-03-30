#pragma once

#include "TaskServiceTableModel.h"
#include <cppmicroservices/BundleContext.h>
#include <kddockwidgets/qtwidgets/views/DockWidget.h>
#include <memory>

QT_BEGIN_NAMESPACE
class QPushButton;
class QTableView;
class QTextEdit;
QT_END_NAMESPACE

class TaskServiceActionDelegate;


class TaskServiceDockWidget : public KDDockWidgets::QtWidgets::DockWidget
{
    Q_OBJECT

public:
    explicit TaskServiceDockWidget(cppmicroservices::BundleContext bundleContext,
                                   QWidget* parent = nullptr);
    ~TaskServiceDockWidget() override;

    void refreshTaskServices();
    void startTaskService(int row);
    void stopTaskService(int row);
    void configureTaskService(int row);

signals:
    void taskServiceStarted(int row);
    void taskServiceStopped(int row);
    void taskServiceConfigured(int row);

private slots:
    void onRefreshTaskServices();
    void onTaskServiceStartStopRequested(int row);
    void onTaskServiceConfigRequested(int row);

private:
    void setupUI();
    void setupConnections();
    int currentTaskServiceRow() const;
    std::shared_ptr<service::ITaskService::IBasicConfig> buildTaskServiceConfig(int row,
                                                                                bool* ok) const;

    cppmicroservices::BundleContext m_bundleContext;

    // UI elements
    QPushButton* m_refreshServicesBtn = nullptr;
    QTableView* m_taskServiceView     = nullptr;

    // Model and delegate
    TaskServiceTableModel* m_taskServiceModel              = nullptr;
    TaskServiceActionDelegate* m_taskServiceActionDelegate = nullptr;
};
