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


class TaskServiceController : public service::ITaskServiceManager::ITaskServiceController,
                              public Thread
{
public:
    TaskServiceController(cppmicroservices::ServiceReference<service::ITaskService> ref,
                          std::shared_ptr<service::ITaskService> service)
        : m_ref(ref)
        , m_service(std::move(service))
    {
    }

    ~TaskServiceController() override { stop(); }

    std::string symbolicName() const override
    {
        try {
            auto b = m_ref.GetBundle();
            return b ? b.GetSymbolicName() : std::string();
        }
        catch (...) {
            return std::string();
        }
    }
    std::string serviceName() const override { return m_service->name(); }
    TaskServiceStatus status() const override
    {
        return isRunning() ? TaskServiceStatus::Running : TaskServiceStatus::Stopped;
    }
    std::shared_ptr<service::ITaskService::IBasicConfig> createConfig() const override
    {
        return m_service->createConfig();
    }
    void setStatusCallback(TaskServiceStatusCallback callback) override
    {
        std::lock_guard lock(m_mutex);
        m_statusCallback = std::move(callback);
    }
    bool start(std::shared_ptr<service::ITaskService::IBasicConfig> config) override
    {
        if (isRunning()) {
            return true;
        }
        {
            std::lock_guard lock(m_mutex);
            m_config = config;
        }
        return Thread::startThread();
    }

    void stop()
    {
        if (isRunning()) {
            m_service->requestStop();
            Thread::stopThread();
        }
    }
    bool isSelf(cppmicroservices::ServiceReference<service::ITaskService> const& reference,
                std::shared_ptr<service::ITaskService> const& service) const
    {
        return m_ref == reference && m_service == service;
    }

private:
    bool onThreadStart() override
    {
        if (!m_service->onThreadStart(m_config))
            return false;

        std::lock_guard lock(m_mutex);
        if (m_statusCallback)
            m_statusCallback(TaskServiceStatus::Running);

        return true;
    }
    bool onThreadRun() override { return m_service->onThreadRun(); }
    void onThreadEnd() override
    {
        m_service->onThreadEnd();

        std::lock_guard lock(m_mutex);
        if (m_statusCallback)
            m_statusCallback(TaskServiceStatus::Stopped);

        m_config.reset();
    }

private:
    cppmicroservices::ServiceReference<service::ITaskService> m_ref;
    std::shared_ptr<service::ITaskService> m_service;
    std::shared_ptr<service::ITaskService::IBasicConfig> m_config;

    mutable std::mutex m_mutex;
    TaskServiceStatusCallback m_statusCallback;
};

class TaskServiceManager : public service::ITaskServiceManager,
                           public cppmicroservices::ServiceTrackerCustomizer<service::ITaskService>
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
