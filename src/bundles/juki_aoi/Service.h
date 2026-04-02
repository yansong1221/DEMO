#pragma once
#include "cppmicroservices/ServiceTracker.h"
#include "imgui.h"
#include "service/IAIAgentService.h"
#include "service/ITaskService.h"

class Service : public service::ITaskService
{
  public:
    Service(cppmicroservices::BundleContext const& context);
    ~Service();

  public:
    std::string name() const override;

    bool onThreadRun() override;

    bool onThreadStart(std::shared_ptr<IBasicConfig> config) override;

    void onThreadEnd() override;

    std::shared_ptr<IBasicConfig> createConfig() const override;

    void requestStop() override;

  private:
    cppmicroservices::ServiceTracker<service::IAIAgentService> m_aiAgentTracker;
};