#include "TaskServiceManager.h"
#include "cppmicroservices/ServiceEvent.h"

TaskServiceController::TaskServiceController(cppmicroservices::ServiceReference<service::ITaskService> ref,
                                             std::shared_ptr<service::ITaskService> service)
    : m_ref(ref)
    , m_service(std::move(service))
{
}

TaskServiceController::~TaskServiceController() { stop(); }

std::string
TaskServiceController::symbolicName() const
{
    try
    {
        auto b = m_ref.GetBundle();
        return b ? b.GetSymbolicName() : std::string();
    }
    catch (...)
    {
        return std::string();
    }
}

std::string
TaskServiceController::serviceName() const
{
    return m_service->name();
}

service::ITaskServiceManager::ITaskServiceController::TaskServiceStatus
TaskServiceController::status() const
{
    return isRunning() ? TaskServiceStatus::Running : TaskServiceStatus::Stopped;
}

std::shared_ptr<service::ITaskService::IBasicConfig>
TaskServiceController::createConfig() const
{
    return m_service->createConfig();
}

void
TaskServiceController::setStatusCallback(TaskServiceStatusCallback callback)
{
    std::lock_guard lock(m_mutex);
    m_statusCallback = std::move(callback);
}

bool
TaskServiceController::start(std::shared_ptr<service::ITaskService::IBasicConfig> config)
{
    if (isRunning())
    {
        return true;
    }
    {
        std::lock_guard lock(m_mutex);
        m_config = config;
    }
    return Thread::startThread();
}

void
TaskServiceController::stop()
{
    if (isRunning())
    {
        m_service->requestStop();
        Thread::stopThread();
    }
}

bool
TaskServiceController::isSelf(cppmicroservices::ServiceReference<service::ITaskService> const& reference,
                              std::shared_ptr<service::ITaskService> const& service) const
{
    return m_ref == reference && m_service == service;
}

std::string
TaskServiceController::displayServiceName() const
{
    return m_service->displayName();
}

bool
TaskServiceController::onThreadStart()
{
    if (!m_service->onThreadStart(m_config))
    {
        return false;
    }

    std::lock_guard lock(m_mutex);
    if (m_statusCallback)
    {
        m_statusCallback(TaskServiceStatus::Running);
    }

    return true;
}

bool
TaskServiceController::onThreadRun()
{
    return m_service->onThreadRun();
}

void
TaskServiceController::onThreadEnd()
{
    m_service->onThreadEnd();

    std::lock_guard lock(m_mutex);
    if (m_statusCallback)
    {
        m_statusCallback(TaskServiceStatus::Stopped);
    }

    m_config.reset();
}

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
    {
        entry->stop();
    }
}

std::vector<service::ITaskServiceManager::ControllerPtr>
TaskServiceManager::listTaskControllers() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<service::ITaskServiceManager::ControllerPtr> controllers;
    for (auto const& entry : m_services)
    {
        controllers.push_back(entry);
    }
    return controllers;
}

void
TaskServiceManager::setControllerEventCallback(ControllerEventCallback callback)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_controllerEventCallback = std::move(callback);
}

std::shared_ptr<service::ITaskService>
TaskServiceManager::AddingService(cppmicroservices::ServiceReference<service::ITaskService> const& reference)
{
    auto svc = m_context.GetService(reference);
    if (!svc)
    {
        return nullptr;
    }
    auto controller = std::make_shared<TaskServiceController>(reference, svc);
    std::lock_guard<std::mutex> lock(m_mutex);
    m_services.push_back(controller);
    if (m_controllerEventCallback)
    {
        m_controllerEventCallback(controller, ControllerEvent::Registered);
    }

    return svc;
}

void
TaskServiceManager::ModifiedService(cppmicroservices::ServiceReference<service::ITaskService> const& reference,
                                    std::shared_ptr<service::ITaskService> const& service)
{
}

void
TaskServiceManager::RemovedService(cppmicroservices::ServiceReference<service::ITaskService> const& reference,
                                   std::shared_ptr<service::ITaskService> const& service)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::find_if(m_services.begin(),
                           m_services.end(),
                           [&reference, &service](std::shared_ptr<TaskServiceController> const& entry)
                           { return entry->isSelf(reference, service); });
    if (it == m_services.end())
    {
        return;
    }

    (*it)->stop();

    if (m_controllerEventCallback)
    {
        m_controllerEventCallback(*it, ControllerEvent::Unregistered);
    }

    m_services.erase(it);
}
