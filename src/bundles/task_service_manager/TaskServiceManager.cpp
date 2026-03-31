#include "TaskServiceManager.h"
#include "cppmicroservices/ServiceEvent.h"

TaskServiceManager::TaskServiceManager(cppmicroservices::BundleContext context)
    : m_context(context)
    , m_Tracker(context, this)
{
    m_Tracker.Open();
}

TaskServiceManager::~TaskServiceManager()
{
    m_Tracker.Close();

    for (auto& entry : m_services)
        entry->stop();
}


std::vector<service::ITaskServiceManager::ControllerPtr>
TaskServiceManager::listTaskControllers() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<service::ITaskServiceManager::ControllerPtr> controllers;
    for (auto const& entry : m_services) {
        controllers.push_back(entry);
    }
    return controllers;
}

void TaskServiceManager::setControllerEventCallback(ControllerEventCallback callback)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_controllerEventCallback = std::move(callback);
}

std::shared_ptr<service::ITaskService> TaskServiceManager::AddingService(
    cppmicroservices::ServiceReference<service::ITaskService> const& reference)
{
    auto svc = m_context.GetService(reference);
    if (!svc) {
        return nullptr;
    }
    auto controller = std::make_shared<TaskServiceController>(reference, svc);
    std::lock_guard<std::mutex> lock(m_mutex);
    m_services.push_back(controller);
    if (m_controllerEventCallback)
        m_controllerEventCallback(controller, ControllerEvent::Registered);

    return svc;
}

void TaskServiceManager::ModifiedService(
    cppmicroservices::ServiceReference<service::ITaskService> const& reference,
    std::shared_ptr<service::ITaskService> const& service)
{
}

void TaskServiceManager::RemovedService(
    cppmicroservices::ServiceReference<service::ITaskService> const& reference,
    std::shared_ptr<service::ITaskService> const& service)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it =
        std::find_if(m_services.begin(),
                     m_services.end(),
                     [&reference, &service](std::shared_ptr<TaskServiceController> const& entry) {
                         return entry->isSelf(reference, service);
                     });
    if (it == m_services.end())
        return;

    if (m_controllerEventCallback)
        m_controllerEventCallback(*it, ControllerEvent::Unregistered);

    (*it)->stop();
    m_services.erase(it);
}