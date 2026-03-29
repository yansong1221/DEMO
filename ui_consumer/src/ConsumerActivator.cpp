#include "demo/IWidgetService.h"
#include <QLabel>
#include <cppmicroservices/BundleActivator.h>
#include <cppmicroservices/BundleContext.h>
#include <cppmicroservices/ServiceTracker.h>
#include <iostream>

using namespace cppmicroservices;


class UiImpl : public demo::IWidgetPlus
{
public:
    QWidget* createWidget(QWidget* parent) override { return new QLabel("Hello from the widget service!",parent); }
    void destroyWidget(QWidget* widget) override { delete static_cast<QLabel*>(widget); }

    void hello() override { throw std::logic_error("The method or operation is not implemented."); }
};

namespace demo {
class ConsumerActivator : public BundleActivator
{
    // std::unique_ptr<ServiceTracker<demo::IGreetingService>> m_tracker;
public:
    void Start(BundleContext context) override
    {
        m_service = std::make_shared<UiImpl>();
        m_reg     = context.RegisterService<demo::IWidgetService>(m_service);
    }
    void Stop(BundleContext) override
    {
        m_reg.Unregister();
        m_service.reset();
    }

private:
    cppmicroservices::ServiceRegistration<demo::IWidgetService> m_reg;
    std::shared_ptr<demo::IWidgetService> m_service;
};
} // namespace demo
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(demo::ConsumerActivator)
