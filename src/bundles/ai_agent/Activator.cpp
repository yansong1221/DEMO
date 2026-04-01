#include "Service.h"
#include "common/Logger.h"

#include "cppmicroservices/BundleActivator.h"
#include <cppmicroservices/ServiceProperties.h>

using namespace cppmicroservices;

class Activator : public cppmicroservices::BundleActivator
{
  public:
    void
    Start(cppmicroservices::BundleContext context) override
    {
        common::Log::init(context);

        m_service = std::make_shared<Service>();
        m_regTaskService = context.RegisterService<service::ITaskService>(m_service);
        m_regAIAgentService = context.RegisterService<service::IAIAgentService>(m_service);
    }
    void
    Stop(cppmicroservices::BundleContext context) override
    {
        m_regTaskService.Unregister();
        m_regAIAgentService.Unregister();
        m_service.reset();

        common::Log::reset();
    }

  private:
    std::shared_ptr<Service> m_service;
    cppmicroservices::ServiceRegistration<service::ITaskService> m_regTaskService;
    cppmicroservices::ServiceRegistration<service::IAIAgentService> m_regAIAgentService;
};

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(Activator)
