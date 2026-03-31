#pragma once

#include "cppmicroservices/BundleContext.h"
#include "service/ITaskService.h"
#include "service/ITaskServiceManager.h"
#include <QAbstractTableModel>
#include <QString>
#include <cppmicroservices/ListenerToken.h>
#include <memory>
#include <vector>

namespace cppmicroservices {
class BundleContext;
}

class TaskServiceTableModel : public QAbstractTableModel
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

    explicit TaskServiceTableModel(cppmicroservices::BundleContext bundleContext,
                                   QObject* parent = nullptr);
    ~TaskServiceTableModel() override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    void refresh();

    bool attachListener();
    void detachListener();

    bool isRunning(int row) const;
    bool startService(int row);
    void stopService(int row);

private:
    void onControllerEvent(std::shared_ptr<service::ITaskServiceManager::ITaskServiceController> controller,
                          service::ITaskServiceManager::ControllerEvent event);
    void onStatusChanged(int index, service::ITaskServiceManager::ITaskServiceController::TaskServiceStatus status);
    void refreshControllers();

    cppmicroservices::BundleContext m_bundleContext;
    std::shared_ptr<service::ITaskServiceManager> m_taskServiceManager;
    std::vector<std::shared_ptr<service::ITaskServiceManager::ITaskServiceController>> m_controllers;
    QString m_lastSelectedName;
    QString m_lastSelectedBundle;
};
