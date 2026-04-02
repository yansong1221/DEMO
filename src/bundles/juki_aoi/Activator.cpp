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

        m_service = std::make_shared<Service>(context);
        m_reg = context.RegisterService<service::ITaskService>(m_service);
    }
    void
    Stop(cppmicroservices::BundleContext context) override
    {
        m_reg.Unregister();
        m_service.reset();

        common::Log::reset();
    }

  private:
    std::shared_ptr<service::ITaskService> m_service;
    cppmicroservices::ServiceRegistration<service::ITaskService> m_reg;
};

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(Activator)
