#pragma once
#include <cppmicroservices/BundleActivator.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/ServiceRegistration.h>
#include "service/ITaskService.h"
#include <memory>

namespace demo {
using service::ITaskService;

class GreetingActivator : public cppmicroservices::BundleActivator {
public:
    void Start(cppmicroservices::BundleContext context) override;
    void Stop(cppmicroservices::BundleContext context) override;

private:
    std::shared_ptr<ITaskService> m_service;
    cppmicroservices::ServiceRegistration<ITaskService> m_reg;
};
}
