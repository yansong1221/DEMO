#include "Service.h"

Service::Service(cppmicroservices::BundleContext const& context) : m_aiAgentTracker(std::move(context))
{
    m_aiAgentTracker.Open();
}

Service::~Service() { m_aiAgentTracker.Close(); }

std::string
Service::name() const
{
    return "juki-aoi";
}

bool
Service::onThreadRun()
{
    return true;
}

bool
Service::onThreadStart(std::shared_ptr<IBasicConfig> config)
{
    return true;
}

void
Service::onThreadEnd()
{
    return;
}

std::shared_ptr<service::ITaskService::IBasicConfig>
Service::createConfig() const
{
    return std::make_shared<DemoTaskConfig>();
}
void
Service::requestStop()
{
    return;
}