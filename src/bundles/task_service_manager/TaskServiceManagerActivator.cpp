#include "TaskServiceManager.h"
#include "common/Logger.h"

#include <cppmicroservices/BundleActivator.h>
#include <cppmicroservices/ServiceEvent.h>
#include <cppmicroservices/ServiceProperties.h>
#include <cppmicroservices/ServiceRegistration.h>

using namespace cppmicroservices;

namespace task_service_manager
{

    class TaskServiceManagerActivator : public BundleActivator
    {
      public:
        void
        Start(BundleContext context) override
        {
            common::Log::init(context);

            m_manager = std::make_shared<TaskServiceManager>(context);
            ServiceProperties props;
            props["service.description"] = std::string("TaskServiceManager bundle service");
            m_registration = context.RegisterService<service::ITaskServiceManager>(m_manager, props);
            common::Log::info("TaskServiceManager started and service registered.");
        }

        void
        Stop(BundleContext) override
        {
            common::Log::info("TaskServiceManager stopped.");

            m_registration.Unregister();
            m_manager.reset();

            common::Log::reset();
        }

      private:
        std::shared_ptr<TaskServiceManager> m_manager;
        ServiceRegistration<service::ITaskServiceManager> m_registration;
    };

} // namespace task_service_manager

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(task_service_manager::TaskServiceManagerActivator)
