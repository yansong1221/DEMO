#pragma once

#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/ServiceTracker.h"
#include "service/ITaskService.h"
#include "service/ITaskServiceManager.h"
#include <QAbstractTableModel>
#include <QString>
#include <cppmicroservices/ListenerToken.h>
#include <memory>
#include <vector>

namespace cppmicroservices
{
    class BundleContext;
}

class TaskServiceTableModel
    : public QAbstractTableModel
    , public cppmicroservices::ServiceTrackerCustomizer<service::ITaskServiceManager>
{
    Q_OBJECT

  public:
    enum Column
    {
        ColName = 0,
        ColBundle,
        ColStatus,
        ColActions,
        ColCount
    };

    explicit TaskServiceTableModel(cppmicroservices::BundleContext bundleContext, QObject* parent = nullptr);
    ~TaskServiceTableModel() override;

    int rowCount(QModelIndex const& parent = QModelIndex()) const override;
    int columnCount(QModelIndex const& parent = QModelIndex()) const override;
    Qt::ItemFlags flags(QModelIndex const& index) const override;
    QVariant data(QModelIndex const& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void refresh();

    bool attachListener();
    void detachListener();

    bool isRunning(int row) const;
    bool startService(int row);
    void stopService(int row);

    std::shared_ptr<service::ITaskService::IBasicConfig> createConfig(int row);
    void configureService(int row, std::shared_ptr<service::ITaskService::IBasicConfig> config);

  protected:
    std::shared_ptr<service::ITaskServiceManager> AddingService(
        cppmicroservices::ServiceReference<service::ITaskServiceManager> const& reference) override;

    void ModifiedService(cppmicroservices::ServiceReference<service::ITaskServiceManager> const& reference,
                         std::shared_ptr<service::ITaskServiceManager> const& service) override;

    void RemovedService(cppmicroservices::ServiceReference<service::ITaskServiceManager> const& reference,
                        std::shared_ptr<service::ITaskServiceManager> const& service) override;

  private:
    void onControllerEvent(std::shared_ptr<service::ITaskServiceManager::ITaskServiceController> controller,
                           service::ITaskServiceManager::ControllerEvent event);
    void onStatusChanged(int index, service::ITaskServiceManager::ITaskServiceController::TaskServiceStatus status);
    void refreshControllers();

    cppmicroservices::BundleContext m_bundleContext;
    cppmicroservices::ServiceTracker<service::ITaskServiceManager> m_Tracker;

    std::shared_ptr<service::ITaskServiceManager> m_taskServiceManager;
    std::vector<std::shared_ptr<service::ITaskServiceManager::ITaskServiceController>> m_controllers;
    QString m_lastSelectedName;
    QString m_lastSelectedBundle;
};
