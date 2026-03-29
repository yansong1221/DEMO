#pragma once

#include "thread.hpp"

#include "service/IGreetingService.h"
#include <QAbstractTableModel>
#include <QScopedPointer>
#include <QString>
#include <QTextEdit>
#include <atomic>
#include <cppmicroservices/Framework.h>
#include <cppmicroservices/ListenerToken.h>
#include <cppmicroservices/ServiceEvent.h>
#include <memory>
#include <vector>

namespace cppmicroservices {
class Framework;
}

// 前置声明
class TaskServiceTableModel;

// TaskServiceEntry 继承 Thread 和 QObject，每个服务在独立线程中运行
class TaskServiceEntry : public QObject, public Thread
{
    Q_OBJECT

public:
    QString defaultConfigYaml;

    cppmicroservices::ServiceReference<service::ITaskService> m_ref;
    std::shared_ptr<service::ITaskService> service;

    // 线程相关
    std::shared_ptr<service::ITaskService::IBasicConfig> config;

    TaskServiceEntry(QObject* parent = nullptr);
    ~TaskServiceEntry() override;

    // 启动服务线程
    bool startService(std::shared_ptr<service::ITaskService::IBasicConfig> cfg);
    // 停止服务线程
    bool stopService(uint32_t millisecond = 5000);

    QString serviceBundleName() const;

signals:
    // 状态改变信号，用于通知UI更新
    void serviceStarted(TaskServiceEntry* entry);
    void serviceStopped(TaskServiceEntry* entry);

protected:
    // Thread 接口实现
    bool onThreadStart() override;
    bool onThreadRun() override;
    void onThreadEnd() override;

private:
    // 日志相关
    std::shared_ptr<spdlog::logger> m_logger;

    void setupLogger(const std::string& pluginName);
};

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

    explicit TaskServiceTableModel(QObject* parent = nullptr);
    ~TaskServiceTableModel() override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    void setFramework(cppmicroservices::Framework* framework);
    void refresh();

    bool attachListener();
    void detachListener();

    TaskServiceEntry* entryAt(int row);
    const TaskServiceEntry* entryAt(int row) const;

    int indexOfEntryRow(const TaskServiceEntry* entry) const;

    bool isRunning(int row) const;
    QString configYaml(int row) const;

signals:
    void serviceLog(QString const& message);

private:
    void onServiceEvent(cppmicroservices::ServiceEvent const& evt);
    void handleServiceRegistered(cppmicroservices::ServiceReferenceBase const& ref);
    void handleServiceUnregistering(cppmicroservices::ServiceReferenceBase const& ref);
    void handleServiceModified(cppmicroservices::ServiceReferenceBase const& ref);
    int findEntryIndexByServiceReference(
        cppmicroservices::ServiceReference<service::ITaskService> const& ref) const;

    QString defaultTaskConfigYaml(std::shared_ptr<service::ITaskService> const& service) const;

    cppmicroservices::Framework* m_framework = nullptr;
    cppmicroservices::ListenerToken m_listenerToken;
    std::vector<std::unique_ptr<TaskServiceEntry>> m_entries;
    QString m_lastSelectedName;
    QString m_lastSelectedBundle;
};
