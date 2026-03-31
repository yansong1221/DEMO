#include "Service.h"

std::string Service::name() const
{
    return "juki";
}

bool Service::onThreadRun()
{
    return true;
}

bool Service::onThreadStart(std::shared_ptr<IBasicConfig> config)
{
    return true;
}

void Service::onThreadEnd()
{
    return;
}

std::shared_ptr<service::ITaskService::IBasicConfig> Service::createConfig() const
{
    return std::make_shared<DemoTaskConfig>();
}
void Service::requestStop()
{
    return;
}