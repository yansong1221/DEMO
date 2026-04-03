#pragma once

#include "cppmicroservices/Bundle.h"
#include "cppmicroservices/ServiceReferenceBase.h"
#include "cppmicroservices/ServiceTracker.h"
#include "cppmicroservices/ServiceTrackerCustomizer.h"
#include "service/ITaskService.h"
#include "service/ITaskServiceManager.h"
#include "thread.hpp"
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/ServiceReference.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class TaskServiceController
    : public service::ITaskServiceManager::ITaskServiceController
    , public Thread
{
  public:
    TaskServiceController(cppmicroservices::ServiceReference<service::ITaskService> ref,
                          std::shared_ptr<service::ITaskService> service);

    ~TaskServiceController() override;

    std::string symbolicName() const override;
    std::string serviceName() const override;
    TaskServiceStatus status() const override;
    std::shared_ptr<service::ITaskService::IBasicConfig> createConfig() const override;

    void setStatusCallback(TaskServiceStatusCallback callback) override;
    bool start(std::shared_ptr<service::ITaskService::IBasicConfig> config) override;

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
    std::shared_ptr<service::ITaskService::IBasicConfig> m_config;

    mutable std::mutex m_mutex;
    TaskServiceStatusCallback m_statusCallback;
};

class TaskServiceManager
    : public service::ITaskServiceManager
    , public cppmicroservices::ServiceTrackerCustomizer<service::ITaskService>
{
  public:
    explicit TaskServiceManager(cppmicroservices::BundleContext context);
    ~TaskServiceManager() override;

    std::vector<ControllerPtr> listTaskControllers() const override;
    void setControllerEventCallback(ControllerEventCallback callback) override;

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

    std::vector<std::shared_ptr<TaskServiceController>> m_services;
    ControllerEventCallback m_controllerEventCallback;
};
