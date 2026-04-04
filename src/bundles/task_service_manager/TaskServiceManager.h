#pragma once

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/ServiceReferenceBase.h"
#include "cppmicroservices/ServiceTracker.h"
#include "cppmicroservices/ServiceTrackerCustomizer.h"
#include "imgui_extend/imguial_msgbox.h"
#include "imgui_extend/table_view.h"
#include "service/ITaskService.h"
#include "service/ITaskServiceManager.h"
#include "thread.hpp"
#include <QObject>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/ServiceReference.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class TaskServiceControllerImpl
    : public service::ITaskServiceManager::ITaskServiceController
    , public Thread
    , public std::enable_shared_from_this<TaskServiceControllerImpl>
{
  public:
    TaskServiceControllerImpl(cppmicroservices::ServiceReference<service::ITaskService> ref,
                              std::shared_ptr<service::ITaskService> service);

    ~TaskServiceControllerImpl() override;

    std::string symbolicName() const override;
    std::string serviceName() const override;
    TaskServiceStatus status() const override;
    std::shared_ptr<service::ITaskService::IBasicConfig> createConfig() const override;
    void saveConfig(std::shared_ptr<service::ITaskService::IBasicConfig> config) override;

    void setStatusCallback(TaskServiceStatusCallback callback) override;
    bool start() override;

    void stop();
    bool isSelf(cppmicroservices::ServiceReference<service::ITaskService> const& reference,
                std::shared_ptr<service::ITaskService> const& service) const;

    std::string displayServiceName() const override;

  private:
    bool onThreadStart() override;
    bool onThreadRun() override;
    void onThreadEnd() override;

  private:
    cppmicroservices::ServiceReference<service::ITaskService> m_ref;
    std::shared_ptr<service::ITaskService> m_service;

    mutable std::mutex m_mutex;
    TaskServiceStatusCallback m_statusCallback;
};

class TaskServiceManagerImpl
    : public QObject
    , public service::ITaskServiceManager
    , public ImGui::extend::TableView
    , public cppmicroservices::ServiceTrackerCustomizer<service::ITaskService>
{
    Q_OBJECT
  public:
    explicit TaskServiceManagerImpl(cppmicroservices::BundleContext context);
    ~TaskServiceManagerImpl() override;

    std::vector<ControllerPtr> listTaskControllers() const override;
    void setControllerEventCallback(ControllerEventCallback callback) override;

    void drawImGui() override;

  protected:
    enum Column
    {
        ColName = 0,
        ColBundle,
        ColStatus,
        ColActions,
        ColCount
    };

    int columnCount() const override;
    std::string headerLable(int column) const override;
    int rowCount() const override;
    void drawCell(int row, int column) override;

  protected:
    std::shared_ptr<service::ITaskService> AddingService(
        cppmicroservices::ServiceReference<service::ITaskService> const& reference) override;

    void ModifiedService(cppmicroservices::ServiceReference<service::ITaskService> const& reference,
                         std::shared_ptr<service::ITaskService> const& service) override;

    void RemovedService(cppmicroservices::ServiceReference<service::ITaskService> const& reference,
                        std::shared_ptr<service::ITaskService> const& service) override;

  private:
    cppmicroservices::BundleContext m_context;
    cppmicroservices::ServiceTracker<service::ITaskService> m_Tracker;

    mutable std::mutex m_mutex;

    std::vector<std::shared_ptr<TaskServiceControllerImpl>> m_services;
    ControllerEventCallback m_controllerEventCallback;

    std::shared_ptr<service::ITaskService::IBasicConfig> currentConfig_;
    std::shared_ptr<TaskServiceControllerImpl> currentController_;

    ImGuiAl::MsgBox restartMsgBox_;
};
