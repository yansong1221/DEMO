#include "Service.h"
#include "common/Logger.h"
#include "trust_mode.h"

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
        m_regTaskService = context.RegisterService<service::ITaskService>(m_service);

        m_programTrustMode = std::make_shared<ProgramTrustModeDrawer>();
        m_regImGuiDrawService = context.RegisterService<service::IImGuiDrawService>(m_programTrustMode);
        m_regTrustProgramService = context.RegisterService<service::ITrustProgramService>(m_programTrustMode);
    }
    void
    Stop(cppmicroservices::BundleContext context) override
    {
        m_regTaskService.Unregister();
        m_regImGuiDrawService.Unregister();
        m_regTrustProgramService.Unregister();

        m_service.reset();
        m_programTrustMode.reset();

        common::Log::reset();
    }

  private:
    std::shared_ptr<Service> m_service;
    std::shared_ptr<ProgramTrustModeDrawer> m_programTrustMode;

    cppmicroservices::ServiceRegistration<service::ITaskService> m_regTaskService;
    cppmicroservices::ServiceRegistration<service::IImGuiDrawService> m_regImGuiDrawService;
    cppmicroservices::ServiceRegistration<service::ITrustProgramService> m_regTrustProgramService;
};

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(Activator)
